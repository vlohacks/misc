#include "loader_stm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "io.h"

/* checks if given data is a s3m, returns 1 if data is valid 
 */
int loader_stm_check(io_handle_t * h)
{
    char signature[8];
    char id;
    size_t saved_pos;
    
    saved_pos = h->tell(h);
    h->seek(h, 0x14, io_seek_set);
    h->read(signature, 1, 8, h);
    h->read(&id, 1, 1, h);
    h->seek(h, saved_pos, io_seek_set);
    
    if (memcmp(signature, "!Scream!", 8))
        return 0;
    
    if (id != 0x1a)
        return 0;

    return 1;
}

/* loads a scream tracker module (stm)
 */
module_t * loader_stm_load(io_handle_t * h)
{
    char signature[8];
    uint8_t pdata[4];
    int r, i, j, k;
    uint16_t tmp_u16;
    uint8_t tmp_u8;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    
    if (!module) 
        return 0;
    
    
    module->module_type = module_type_stm;

    /* chech if we really deal with a STM file. IF not, bail out */
    h->seek(h, 0x14, io_seek_set);
    r = h->read(signature, 1, 8, h);
    
    if (r != 8) {
        free(module);
        return 0;
    }
    
    if (memcmp(signature, "!Scream!", 8)) {
        free(module);
        return 0;
    }
    
    r = h->read(&tmp_u8, 1, 1, h);
    if (r != 1) {
        free(module);
        return 0;
    }
    
    if (tmp_u8 != 0x1a) {
        free(module);
        return 0;
    }
    
    
    module->initial_bpm = 125;
    module->initial_speed = 6;
    module->num_samples = 31;
    module->num_channels = 4;
    
    /* read the song name */
    h->seek(h, 0, io_seek_set);
    r = h->read(module->song_title, 1, 20, h);

    /* skip signature and ID byte */
    h->seek(h, 9, io_seek_cur);

    /* file type */
    r = h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    
    /* version infos */
    r = h->read(&(module->module_info.flags_stm.version_major), 1, 1, h);
    r = h->read(&(module->module_info.flags_stm.version_minor), 1, 1 ,h);

    /* initial_tempo */
    r = h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    
    
    r = h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    module->num_patterns = (uint16_t)tmp_u8;
    
    /* global volume - TODO, currently IGNORED */
    r = h->read (&tmp_u8, 1, 1, h);
    
    /* skip 13 "reserved" bytes */
    h->seek(h, 13, io_seek_cur);
        
    
    /* read sample headers */
    module->samples = (module_sample_t *)malloc(sizeof(module_sample_t) * module->num_samples);
    for (i = 0; i < module->num_samples; i++) {
        h->read(module->samples[i].header.name, 1, 12, h);
        module->samples[i].header.name[12] = 0;
        /* skip ID, instrument disk, reserved(word) */
        h->seek(h, 4, io_seek_cur);
        
        h->read(&tmp_u16, sizeof(uint16_t), 1, h);
        module->samples[i].header.length = (uint32_t)tmp_u16;
        h->read(&tmp_u16, sizeof(uint16_t), 1, h);
        module->samples[i].header.loop_start = (uint32_t)tmp_u16;
        if (tmp_u16 > 0)
            module->samples[i].header.loop_enabled = 1;
        else 
            module->samples[i].header.loop_enabled = 0;
        h->read(&tmp_u16, sizeof(uint16_t), 1, h);
        module->samples[i].header.loop_end = (uint32_t)tmp_u16;
        module->samples[i].header.loop_length = module->samples[i].header.loop_end - module->samples[i].header.loop_start;
        
        
        
        h->read(&tmp_u8, sizeof(uint8_t), 1, h);
        module->samples[i].header.volume = tmp_u8;
        
        /* skip "reserved" byte */
        h->seek(h, 1, io_seek_cur);

        h->read(&tmp_u16, sizeof(uint16_t), 1, h);
        module->samples[i].header.c2spd = tmp_u16;

        /* skip "reserved" dword an "length in paragraphs" word */
        h->seek(h, 6, io_seek_cur);
    }
    
    /* read orders */
    for (i = 0; i < 128; i++) {
        h->read(&tmp_u8, sizeof(uint8_t), 1, h);
        module->orders[i] = tmp_u8;
    }
    
    /* determine num orders */
    for (i = 0; i < 128; i++) {
        if (module->orders[i] >= 0x63) {
            module->num_orders = i;
            break;
        }
    }
    
    /* read pattren data */
    module->patterns = (module_pattern_t *)malloc(sizeof(module_pattern_t) * module->num_patterns);
    for (i = 0; i < module->num_patterns; i++) {
        module->patterns[i].num_rows = 64;
        module->patterns[i].rows = (module_pattern_row_t *)malloc(sizeof(module_pattern_row_t) * 64);
        for (j = 0; j < 64; j++) {
            module->patterns[i].rows[j].data = (module_pattern_data_t *)malloc(sizeof(module_pattern_data_t) * module->num_channels);
            for (k = 0; k < module->num_channels; k++) {
                module_pattern_data_t * d = &(module->patterns[i].rows[j].data[k]);
                h->read(&pdata, 4, 1, h);
                d->period_index = -1;
                
                if (pdata[0] == 252) {
                    d->period_index = 254;
                } else if (pdata[0] < 251) {
                    //low nibble = note, high nibble = octave
                    d->period_index = (int)(((pdata[0] >> 4) * 12) + (pdata[0] & 0x0f));    
                }
                
                d->sample_num = pdata[1] >> 3;
                tmp_u8 = (pdata[1] & 7) | ((pdata[2] & 0xf0) >> 1);
                d->volume = tmp_u8 > 64 ? -1 : tmp_u8;
                
                d->effect_num = pdata[2] & 0x0f;
                d->effect_value = pdata[3];
                        
            }
        }
    }
    
    for (i = 0; i < 31; i++) {
        
        if (module->samples[i].header.length) {
            module->samples[i].data = (int8_t *)malloc(module->samples[i].header.length);
            
            h->read(module->samples[i].data, module->samples[i].header.length, 1, h);
            
            // in stm samples are aligned / padded to 16 byte bounds
            int align = (module->samples[i].header.length  + 15) & ~15;
            h->seek(h, align - module->samples[i].header.length, io_seek_cur);

        } else {
            module->samples[i].data = 0;
        }
              
    }
    
    for (i = 0; i < 4; i++) {
        module->initial_panning[i] = (i & 1) ? 0x0 : 0xff;
    }
    
    size_t tmps = h->tell(h);
    printf("pos: %u\n", tmps);
    
    h->seek(h, 0, io_seek_end);
    tmps = h->tell(h);
    printf("len: %u\n", tmps);
    
    
    
    //module_dump(module, stdout);
    module->song_message = 0;
    return module;
}

