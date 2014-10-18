/* 
 * File:   output_dos_sb16.h
 * Author: vlo
 * 
 * 1337 DOS SB16 Output - yeah
 *
 * Created on 18. Oktober 2014, 22:48
 */

#include <stdio.h>
#include <dos.h>
#include <malloc.h>
#include <math.h>
#include <mem.h>

#include <stdlib.h>
#include <string.h>
#include <sys\movedata.h>
#include <sys\nearptr.h>
#include <pc.h>
#include <go32.h>
#include <dpmi.h>

#include "output.h"
#include "player.h"

#ifndef OUTPUT_SB16_H
#define	OUTPUT_SB16_H

#define OUTPUT_SB16_DSP_RESET 0x06
#define OUTPUT_SB16_DSP_DATAAVAIL 0x0e
#define OUTPUT_SB16_DSP_READ 0x0a
#define OUTPUT_SB16_DSP_OUTPUT 0x41

typedef enum {
    OUTPUT_SB16_RESOLUTION_8BIT = 0,
    OUTPUT_SB16_RESOLUTION_16BIT = 1
} output_sb16_resolution_t;

typedef enum {
    OUTPUT_SB16_CHANNELS_MONO = 0,
    OUTPUT_SB16_CHANNELS_STEREO = 1,
} output_sb16_channels_t;

typedef struct {
    unsigned int port;
    unsigned int irq;
    unsigned int dma;
    unsigned int hdma;
    unsigned int dma_buffer_size;
    unsigned int sample_rate;
    output_sb16_resolution_t resolution;
    output_sb16_channels_t channels;
    
} output_sb16_settings_t;

typedef struct {
    void *              dma_buffer;
    _go32_dpmi_seginfo  dma_buffer_dos;
    int                 dma_buffer_dos_offset;    
    _go32_dpmi_seginfo  old_irq;
    _go32_dpmi_seginfo  my_irq;
    unsigned int irq;
    unsigned int dma;
    unsigned int dma_chunk_size;
    unsigned int dma_buffer_current_chunk;
    player_t * player;
} output_sb16_env_t;

int output_sb16_init(output_opts_t * output_opts);
int output_sb16_cleanup();
int output_sb16_start(player_t * player);
int output_sb16_stop();

void output_sb16_wait();


#endif	/* OUTPUT_SB16_H */

