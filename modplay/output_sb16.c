#include "output_sb16.h"
#include "player.h"
#include "mixing.h"
#include <stdint.h>

output_sb16_settings_t output_sb16_settings;
output_sb16_env_t output_sb16_env;

void output_sb16_dsp_write(unsigned int base_port, unsigned char val) {
  while ((inportb(base_port + 0xc) & 0x80) == 0x80);
  outportb (base_port + 0xc, val);
}

int output_sb16_dsp_reset(unsigned int base_port) {
    unsigned int res;
    outportb(base_port + OUTPUT_SB16_DSP_RESET, 0x01);
    delay(10);
    outportb(base_port + OUTPUT_SB16_DSP_RESET, 0x00);
    delay(10);
    res = inportb(base_port + OUTPUT_SB16_DSP_DATAAVAIL);
    if (res & 0x80) {
        res = inportb(base_port + OUTPUT_SB16_DSP_READ);
        if (res == 0xaa) {
            return 0;
        } else {
            return -1;
        }
    }
}


int output_sb16_dsp_start(output_sb16_env_t * env, unsigned int base_port, unsigned int sample_rate, output_sb16_resolution_t resolution, output_sb16_channels_t channels) {
    output_sb16_dsp_write(base_port, 0x41);    // output mode
    output_sb16_dsp_write(base_port, sample_rate >> 8);    
    output_sb16_dsp_write(base_port, sample_rate & 0xff);  
    
    unsigned int chunk_size = env->dma_chunk_size;
    
    if (resolution == OUTPUT_SB16_RESOLUTION_8BIT) {
        output_sb16_dsp_write(base_port, 0xc6);
        env->sample_size = 1;
        switch (channels) {
            case OUTPUT_SB16_CHANNELS_MONO:   output_sb16_dsp_write(base_port, 0x00); break;
            case OUTPUT_SB16_CHANNELS_STEREO: output_sb16_dsp_write(base_port, 0x20); break;
        }
    } else {
        output_sb16_dsp_write(base_port, 0xb6);
        env->sample_size = 2;
        switch (channels) {
            case OUTPUT_SB16_CHANNELS_MONO:   output_sb16_dsp_write(base_port, 0x10); break;
            case OUTPUT_SB16_CHANNELS_STEREO: output_sb16_dsp_write(base_port, 0x30); break;
        }
        chunk_size >>= 1;
    }
    
    output_sb16_dsp_write(base_port, (chunk_size - 1) & 0xff);  
    output_sb16_dsp_write(base_port, (chunk_size - 1) >> 8);
    
    return 0;
}

void output_sb16_dsp_stop(unsigned int base_port, output_sb16_resolution_t resolution) {
    if (resolution == OUTPUT_SB16_RESOLUTION_8BIT) {
        output_sb16_dsp_write(base_port, 0xda);
    } else {
        output_sb16_dsp_write(base_port, 0xd9);
    }
}

int output_sb16_dma_setup(output_sb16_env_t * env, unsigned int dma, unsigned int dmabuffer_size) {
    int dmabuffer_page;
    int dmabuffer_offset;
    
    dmabuffer_page = env->dma_buffer_dos_offset >> 16;
    dmabuffer_offset = env->dma_buffer_dos_offset & 0xffff;
    
    // 16 bit dma special since it is wired off by one to the mem enabling
    // 16 bit transfers with normal incrementing counter
    if (dma > 3) {
        dmabuffer_offset = (env->dma_buffer_dos_offset >> 1) & 0xffff;
        dmabuffer_size >>= 1;
    }
    // all DMAs above 3 are on the secondary DMA controller starting with 0 again
    unsigned int dma_id = dma & 3;
    
    output_sb16_dma_map_t * dma_map = &(output_sb16_dma_maps[dma]);
    
    outportb(dma_map->mask, 4 | dma_id);               // mask dma
    outportb(dma_map->flipflop_reset, 0);           // reset flip flop
    outportb(dma_map->mode, 0x58 | dma_id);   // 01=block mode on, 00=no single cycle, 10 = read
    
    // write offset
    outportb(dma_map->start_address, dmabuffer_offset & 0xff);
    outportb(dma_map->start_address, dmabuffer_offset >> 8);
    
    // write page
    outportb(dma_map->page_address, dmabuffer_page);

    // write dma buffer length (length - 1, bug?)
    outportb(dma_map->count, (dmabuffer_size - 1) & 0xff);
    outportb(dma_map->count, (dmabuffer_size - 1) >> 8);
    
    outportb(dma_map->mask, dma_id);               // unmask dma
    env->dma = dma;
    
    return 0;
}


void output_sb16_dma_setup_buffers(output_sb16_env_t * env, unsigned int buffer_size) {
    _go32_dpmi_seginfo tmp1, tmp2;
    unsigned int page1, page2;
    
    // chunk size = buffer_size / 2 - double buffering
    env->dma_chunk_size = buffer_size >> 1;
    env->dma_buffer_current_chunk = 0;
    env->dma_buffer = (void *)malloc(env->dma_chunk_size);
    // size in paragraphs
    tmp1.size = buffer_size >> 4;
    _go32_dpmi_allocate_dos_memory(&tmp1);
    
    env->dma_buffer_dos_offset = tmp1.rm_segment << 4;
    
    page1 = env->dma_buffer_dos_offset >> 16;
    page2 = (env->dma_buffer_dos_offset + (buffer_size - 1)) >> 16;
    
    // allocated mem crosses page boundary?
    if (page1 != page2) {
        tmp2.size = buffer_size >> 4;
        _go32_dpmi_allocate_dos_memory(&tmp1);
        env->dma_buffer_dos_offset = tmp2.rm_segment << 4;
        _go32_dpmi_free_dos_memory(&tmp1);
        env->dma_buffer_dos = tmp2;
    } else {
        env->dma_buffer_dos = tmp1;
    }
    
    // initialize dma buffers
    memset(env->dma_buffer, 0, buffer_size);
    dosmemput(env->dma_buffer, buffer_size, env->dma_buffer_dos_offset);
}

void output_sb16_dma_free_buffers(output_sb16_env_t * env) {
    _go32_dpmi_free_dos_memory(&(env->dma_buffer_dos));
    free(env->dma_buffer);
}


/* this is called after every dma transfer
 */
void output_sb16_isr() {
    int i;
    sample_t l, r;
    //inportb (0x22E);
    unsigned int a, b;
    outportb(output_sb16_settings.port + 4, 0x82);
    a = inportb(output_sb16_settings.port + 5);
    if (a & 1)
        b = inportb(output_sb16_settings.port + 0x0e);

    if (a & 2)
        b = inportb(output_sb16_settings.port + 0x0f);

    
    //Acknowledge hardware interrupt
    outportb (0x20, 0x20);
    //Acknowledge cascade interrupt for IRQ 2, 10 and 11 TODO
    //if (IRQ == 2 || IRQ == 10 || IRQ == 11) 
    //    outportb (0xa0, 0x20);

    i = output_sb16_env.dma_chunk_size;
    char * p = (char *)output_sb16_env.dma_buffer;
    int16_t * pp = (int16_t *)p;
    while (i) {
        if (player_read(output_sb16_env.player, &l, &r) != 2) {
            output_sb16_env.player->playing = 0;
        } else {
            if (output_sb16_env.sample_size == 2) {
                *pp++ = sample_to_s16(l);
                *pp++ = sample_to_s16(r);
            } else {
                *p++ = sample_to_s8(l) ^ 128;
                *p++ = sample_to_s8(r) ^ 128;
            }
            i -= output_sb16_env.sample_size;
            i -= output_sb16_env.sample_size;
        }        
    }
    
    if (output_sb16_env.dma_buffer_current_chunk)
        dosmemput(output_sb16_env.dma_buffer, output_sb16_env.dma_chunk_size, output_sb16_env.dma_buffer_dos_offset + output_sb16_env.dma_chunk_size);
    else
        dosmemput(output_sb16_env.dma_buffer, output_sb16_env.dma_chunk_size, output_sb16_env.dma_buffer_dos_offset);
    
    output_sb16_env.dma_buffer_current_chunk++;
    output_sb16_env.dma_buffer_current_chunk &= 1;
    
    
}

void output_sb16_install_isr(output_sb16_env_t * env, unsigned int irq) {
    env->my_irq.pm_offset = (int)output_sb16_isr;
    env->my_irq.pm_selector = _go32_my_cs();
    
    // save original irq vector
    _go32_dpmi_get_protected_mode_interrupt_vector (irq + 8, &(env->old_irq));
    _go32_dpmi_chain_protected_mode_interrupt_vector (irq + 8, &(env->my_irq));
    
    env->irq = irq;
}

void output_sb16_remove_isr(output_sb16_env_t * env, unsigned int irq) {
    _go32_dpmi_set_protected_mode_interrupt_vector (irq + 8, &(env->old_irq));
}





int output_sb16_init(output_opts_t * output_opts)
{
    /* TODO make customizable!!!!!!!!
     */
    output_sb16_settings.port = 0x220;
    output_sb16_settings.irq = 5;
    output_sb16_settings.dma = 1;
    output_sb16_settings.hdma = 5;
    
    output_sb16_settings.sample_rate = output_opts->sample_rate;
    output_sb16_settings.dma_buffer_size = output_opts->buffer_size;
    
    if (output_sb16_dsp_reset(output_sb16_settings.port)) {
        fprintf(stderr, "error resetting the dsp :-/\n");
        return -1;
    }
    
    output_sb16_dma_setup_buffers(&output_sb16_env, output_sb16_settings.dma_buffer_size);
    output_sb16_install_isr(&output_sb16_env, output_sb16_settings.irq);
    output_sb16_dma_setup(&output_sb16_env, output_sb16_settings.hdma, output_sb16_settings.dma_buffer_size);
    
    return 0;
}

int output_sb16_cleanup() {
    output_sb16_remove_isr(&output_sb16_env, output_sb16_settings.irq);
    output_sb16_dma_free_buffers(&output_sb16_env);
    return 0;
}

int output_sb16_start(player_t * player) {
    output_sb16_env.player = player;
    player->playing = 1;
    outportb (0x21, inportb (0x21) & !(1 << output_sb16_settings.irq));
    output_sb16_dsp_start(&output_sb16_env, output_sb16_settings.port, 44100, 1, OUTPUT_SB16_CHANNELS_STEREO);
    return 0;
}

int output_sb16_stop() {
    output_sb16_dsp_stop(output_sb16_settings.port, 1);
    delay(100);
    outportb (0x21, inportb (0x21) | (1 << output_sb16_settings.irq));
    return 0;
}