/* 
 * File:   output_dos_sb16.h
 * Author: vlo
 * 
 * 1337 DOS SB16 Output - yeah
 * TODO: Make irq/dma configurable, parse BLASTER env var, select 8/16 bit 
 * depending on DSP version
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
    unsigned char page_address;
    unsigned char start_address;
    unsigned char count;
    unsigned char mask;
    unsigned char mode;
    unsigned char flipflop_reset;
} output_sb16_dma_map_t;


static const output_sb16_dma_map_t output_sb16_dma_maps[8] = {
    //page  addr  count mask  mode  flip
    { 0x87, 0x00, 0x01, 0x0a, 0x0b, 0x0c },
    { 0x83, 0x02, 0x03, 0x0a, 0x0b, 0x0c },
    { 0x81, 0x04, 0x05, 0x0a, 0x0b, 0x0c },
    { 0x82, 0x06, 0x07, 0x0a, 0x0b, 0x0c },     // DMA 4 is cascaded to slave, therefore unusable
    { 0x8f, 0xc0, 0xc2, 0xd4, 0xd6, 0xd8 },
    { 0x8b, 0xc4, 0xc6, 0xd4, 0xd6, 0xd8 },
    { 0x89, 0xc8, 0xca, 0xd4, 0xd6, 0xd8 },
    { 0x8a, 0xcc, 0xce, 0xd4, 0xd6, 0xd8 },
};

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
    unsigned int sample_size;
    player_t * player;
} output_sb16_env_t;


int output_sb16_init(output_opts_t * output_opts);
int output_sb16_cleanup();
int output_sb16_start(player_t * player);
int output_sb16_stop();

void output_sb16_wait();


#endif	/* OUTPUT_SB16_H */

