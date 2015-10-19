#include "loader_psm_masi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "module_utils.h"
#include "mixing.h"

#include "io.h"

/* checks if given data is a masi PSM, returns 1 if data is valid 
 */
int loader_psm_masi_check(io_handle_t * h)
{
    char signature[4];
    size_t saved_pos;
    
    saved_pos = h->tell(h);
    h->seek(h, 0x0, io_seek_set);
    h->read(signature, 1, 4, h);
    h->seek(h, saved_pos, io_seek_set);
    
    if (memcmp(signature, LOADER_PSM_MASI_HEADER, 4))
        return 0;

    return 1;
}

/* loads a MASI PSM Module
 */
module_t * loader_psm_masi_load(io_handle_t * h)
{
    loader_psm_masi_state_t state;
    
    int i;
    uint8_t tmp8;
    uint32_t tmp32;
    union  {
        uint32_t i;
        char s[5];
    } currentchunk;
    size_t next_chunk_offset;
    
    currentchunk.s[4] = 0;
    
    if (!loader_psm_masi_check(h))
        return 0;

    state.module = malloc(sizeof(module_t));
    state.module->module_type = module_type_s3m;
    state.module->song_message = 0;
    
    /* these are determined in analysis phase */
    state.module->num_channels = 0;
    state.module->num_instruments = 0;
    state.module->num_orders = 0;
    state.module->num_patterns = 0;
    state.module->num_samples = 0;
    
    /* 2 iterations - analysis phase and loading phase 
     * starting with analysis phase
     */
    state.phase = 0;
    for (i = 0; i < 2; i++) {
        next_chunk_offset = 12;
        for(;;) {
            h->seek(h, next_chunk_offset, io_seek_set);
            h->read(&currentchunk, 4, 1, h);
            h->read(&tmp32, sizeof(uint32_t), 1, h);
            
            if (h->feof(h))
                break;
            
            next_chunk_offset = h->tell(h) + tmp32;
            
            //printf("%08x: %s\n", h->tell(h), currentchunk.s);
            
            switch (currentchunk.i) {
                case 0x444f4250:    loader_psm_masi_PBOD(h, &state); break;
                case 0x504d5344:    loader_psm_masi_DSMP(h, &state); break;
                case 0x474e4f53:    loader_psm_masi_SONG(h, &state, tmp32); break;
                default:            break; // TODO: cry if unhandled chunks occur?
            }
        }
        
        /* allocate memory for data which sizes have been determined in anaysis
         * phase
         */
        if (state.phase == 0) {
            state.module->patterns = malloc(sizeof(module_pattern_t) * state.module->num_patterns);
            state.module->samples = malloc(sizeof(module_sample_t) * state.module->num_samples);
        }
        state.phase = 1;
    }

    //module_dump(state.module, stdout);
    
    //exit(0);
    return(state.module);
}


/* translate PSM Effects to S3M effects */
void loader_psm_masi_translate_effect(char * effect_num, char * effect_value) 
{
    switch (*effect_num) {
        case 0x01: 
            *effect_num = 4;
            *effect_value <<= 4;
            *effect_value |= 0x0f;
            break;
        case 0x02: 
            *effect_num = 4;
            *effect_value <<= 4;
            break;
        case 0x03:
            *effect_num = 4;
            *effect_value |= 0xf0;
            break;
        case 0x04:
            *effect_num = 4;
            *effect_value &= 0x0f;
            break;
            
        case 0x0b:
            *effect_num = 6;
            *effect_value >>= 3;
            *effect_value |= 0xf0;
            break;
            
        case 0x0c:
            *effect_num = 6;
            if (*effect_value < 4) {
                *effect_value |= 0xf0;
            } else {
                *effect_value >>= 3;
            }
            break;
            
        case 0x0d:
            *effect_num = 5;
            *effect_value >>= 3;
            *effect_value |= 0xf0;
            break;
            
        case 0x0e:
            *effect_num = 5;
            if (*effect_value < 4) {
                *effect_value |= 0xf0;
            } else {
                *effect_value >>= 3;
            }
            break;
            
        case 0x0f:
            *effect_num = 7;
            *effect_value >>= 3;
            break;
            
        case 0x10:
            *effect_num = 18;
            *effect_value &= 0x1f;
            break;
            
        case 0x11:
            *effect_num = 11;
            *effect_value <<= 4;
            break;

        case 0x12:
            *effect_num = 11;
            *effect_value &= 0x0f;
            break;
            
        case 0x15:
            *effect_num = 7;
            break;
            
        case 0x16:
            *effect_num = 18;
            *effect_value &= 0x30;
            break;
            
        case 0x17:
            *effect_num = 10;
            *effect_value &= 0x0f;
            break;
            
        case 0x18:
            *effect_num = 10;
            *effect_value <<= 4;
            break;
            
        case 0x3d:
            *effect_num = 1;
            break;
            
        default:
            fprintf(stderr, "KILLING UNKNOWN EFFECT: %02x\n", *effect_num);
            *effect_num = 0;
            *effect_value = 0;
            break;
            
    }
}

/* The Order List Subchunk of the SONG chunk */
int loader_psm_masi_SONG_OPLH(io_handle_t * h, loader_psm_masi_state_t * state)
{
    uint16_t num_subchunks;
    uint8_t subchunk;
    uint8_t tmp8;
    uint8_t tmp8_2;
    uint8_t tmp8_3;
    char pattern_id[9];
    
    h->read(&num_subchunks, sizeof(uint16_t), 1, h);
    
    while (num_subchunks) {
        h->read(&subchunk, 1, 1, h);
        switch (subchunk) {
            case 0x00: break;                               // end indicator
            case 0x01:                                      // order list entry 
                h->read(pattern_id, 4, 1, h);
                //printf("====%s====!!!!!\n", pattern_id);
                state->module->orders[state->module->num_orders] = atoi(&pattern_id[1]);
                state->module->num_orders++;
                break;
            case 0x04:h->seek(h, 3, io_seek_cur); break;    // restart chunk .. whatever 
            case 0x07:                                      // default speed
                h->read(&tmp8, 1, 1, h);
                state->module->initial_speed = tmp8;
                break;
            case 0x08:                                      // default speed
                h->read(&tmp8, 1, 1, h);
                state->module->initial_bpm = tmp8;
                break;
            case 0x0c: h->seek(h, 6, io_seek_cur); break;   // sample map table - whatever - skip
            case 0x0d: 
                h->read(&tmp8, 1, 1, h);
                h->read(&tmp8_2, 1, 1, h);
                h->read(&tmp8_3, 1, 1, h);
                state->module->initial_panning[tmp8] = tmp8_2;
                break;
            case 0x0e: h->seek(h, 1, io_seek_cur); break;   // sample map table - whatever - skip
        }
        num_subchunks--;
    }
}

/* SONG - Order list and default tempo/speed are from interest in this chunk */
int loader_psm_masi_SONG(io_handle_t * h, loader_psm_masi_state_t * state, uint32_t size)
{
    uint32_t subsize;
    union {
        uint32_t i;
        char s[5];
    } subchunk;
    
    /* skip type, compression and num_channels which are not from interest */
    h->seek(h, 11, io_seek_cur);
    
    while (size > 0) {
        h->read(&subchunk, sizeof(uint32_t), 1, h);
        h->read(&subsize, sizeof(uint32_t), 1, h);
        
        size -= 8;
        switch (subchunk.i) {
            case 0x484c504f:    loader_psm_masi_SONG_OPLH(h, state); break;
            default:            h->seek(h, subsize, io_seek_cur); break;
        }
        
        if (subsize > size)
            break;
        
        size -= subsize;
    }
}

/* DSMP - Sample data */
int loader_psm_masi_DSMP(io_handle_t * h, loader_psm_masi_state_t * state) 
{
    uint8_t last_sample;
    uint8_t tmp8;
    uint8_t flags;
    uint16_t tmp16;
    int sample_nr;
    uint32_t tmp32;
    int i;
    
    char tmpbuf[33];
    
    h->read(&flags, 1, 1, h);
    
    h->seek(h, 12, io_seek_cur);
    
    h->read(tmpbuf, 33, 1, h);
    h->seek(h, 6, io_seek_cur);
    h->read(&tmp16, sizeof(uint16_t), 1, h);
    
    sample_nr = tmp16;
    
    /* in analyze phase just count the highest sample index */
    if (state->phase == 0) {
        if (state->module->num_samples < (tmp16 + 1)) {
            state->module->num_samples = (tmp16 + 1);
        }
    } else {
        if (flags & (1<<7))
            state->module->samples[sample_nr].header.loop_enabled = 1;
        else 
            state->module->samples[sample_nr].header.loop_enabled = 0;
        
        strncpy(state->module->samples[sample_nr].header.name, tmpbuf, 28);
        
        h->read(&tmp32, sizeof(uint32_t), 1, h);
        state->module->samples[sample_nr].header.length = tmp32;
        
        h->read(&tmp32, sizeof(uint32_t), 1, h);
        state->module->samples[sample_nr].header.loop_start = tmp32;

        h->read(&tmp32, sizeof(uint32_t), 1, h);
        state->module->samples[sample_nr].header.loop_end = tmp32;
        state->module->samples[sample_nr].header.loop_length = state->module->samples[sample_nr].header.loop_end - state->module->samples[sample_nr].header.loop_start;

        h->seek(h, 2, io_seek_cur);
        
        h->read(&tmp8, 1, 1, h);
        state->module->samples[sample_nr].header.volume = (tmp8 >> 1)+1;
        
        h->seek(h, 4, io_seek_cur);
        
        h->read(&tmp16, sizeof(uint16_t), 1, h);
        state->module->samples[sample_nr].header.c2spd = tmp16;
        
        h->seek(h, 21, io_seek_cur);
        
        if (state->module->samples[sample_nr].header.length > 0) {
            state->module->samples[sample_nr].data = malloc(sizeof(sample_t) * state->module->samples[sample_nr].header.length);
            last_sample = 128;
            for (i = 0; i < state->module->samples[sample_nr].header.length; i++) {
                h->read(&tmp8, 1, 1, h);
                last_sample = (last_sample + tmp8);
                
                state->module->samples[sample_nr].data[i] = sample_from_s8(last_sample ^ 0x80);
            }
        } else {
            state->module->samples[sample_nr].data = 0;
        }
        
    }
   
}

/* PBOD - Patten BODy - the Pattern Data */
int loader_psm_masi_PBOD(io_handle_t * h, loader_psm_masi_state_t * state) 
{
    int i, j;
    uint32_t datasize;
    uint16_t rowsize;
    int pattern_nr;
    int row_nr;
    int channel_nr;
    uint16_t tmp16;
    uint8_t tmp8;
    uint8_t tmp8_2;
    uint8_t flags;
    
    char pattern_id[9];
    
    /* PBOD chunk contains it's size TWICE for what reason ever... */
    h->read(&datasize, sizeof(uint32_t), 1, h);
    datasize -= 4;
    /* 
     * The pattern ID is either Pnnn or PATTnnnn, there are 2 different versions
     * of MASI PSMs. TODO: get "Sinaria" - a game where the PATT variant is used
     * to test 
     * notice nnn / nnnn are ASCII representations of the pattern number 
     */
    h->read(pattern_id, 4, 1, h);
    if (memcmp(pattern_id, "PATT", 4)) {
        // Jazz / Pinball variant... Pxxx
        pattern_id[4] = 0;
        pattern_nr = atoi(&pattern_id[1]);
        datasize -= 4;
    } else {
        // Sinaria variant... PATTxxxx
        h->read(&pattern_id[4], 4, 1, h);
        pattern_id[9] = 0;
        pattern_nr = atoi(&pattern_id[4]);
        datasize -= 8;
    }
    
    if (state->phase == 0) {
        /* in analysis phase just determine the pattern count */
        if (state->module->num_patterns < ((uint16_t)pattern_nr +1))
            state->module->num_patterns = ((uint16_t)pattern_nr +1);
    }
    /* num rows in pattern */
    h->read(&tmp16, sizeof(uint16_t), 1, h);
    datasize -= 2;

    
    
    /* allocate memory for this pattern in loading phase */
    if (state->phase == 1) {
        state->module->patterns[pattern_nr].num_rows = tmp16;
        state->module->patterns[pattern_nr].rows = malloc(sizeof(module_pattern_row_t) * state->module->patterns[pattern_nr].num_rows);
        for (i = 0; i < state->module->patterns[pattern_nr].num_rows; i++) {
            state->module->patterns[pattern_nr].rows[i].data = malloc(sizeof(module_pattern_data_t) * state->module->num_channels);
            for (j = 0; j < state->module->num_channels; j++) {
                state->module->patterns[pattern_nr].rows[i].data[j].effect_num = 0;
                state->module->patterns[pattern_nr].rows[i].data[j].effect_value = 0;
                state->module->patterns[pattern_nr].rows[i].data[j].period_index = -1;
                state->module->patterns[pattern_nr].rows[i].data[j].sample_num = 0;
                state->module->patterns[pattern_nr].rows[i].data[j].volume = -1;
            }
        }
    }

    row_nr = 0;
    while (datasize > 0) {
        /* read flags which determine what data follows 
         * (1<<7) : note
         * (1<<6) : instrument
         * (1<<5) : volume
         * (1<<4) : effect num and parameter
         */
        h->read(&rowsize, sizeof(uint16_t), 1, h);
        datasize -= rowsize;
        
        rowsize -= 2;
        
        while (rowsize) {
            h->read(&flags, 1, 1, h);
            rowsize--;
            
            if (rowsize == 1)
                break;

            /* channel number the following data belongs to */
            h->read(&tmp8, 1, 1, h);
            rowsize--;
            channel_nr = tmp8;
            /*
            if (state->phase)
                printf("%i--%s--%i--%i(%i)--%i\n", datasize, pattern_id, pattern_nr, row_nr, state->module->patterns[pattern_nr].num_rows, channel_nr);
            else 
                printf("%i--%s--%i--%i--%i\n", datasize, pattern_id, pattern_nr, row_nr, channel_nr);
             */
            /* determine "ceil" of channels in analysis phase */
            if (state->phase == 0) {
                if (state->module->num_channels < ((uint16_t)tmp8 + 1))
                    state->module->num_channels = ((uint16_t)tmp8 + 1);
            } 

            /* load note */
            if (flags & (1<<7)) {
                h->read(&tmp8, 1, 1, h);
                rowsize--;
                if ((state->phase == 1) && (row_nr < state->module->patterns[pattern_nr].num_rows))
                    state->module->patterns[pattern_nr].rows[row_nr].data[channel_nr].period_index = (tmp8 >> 4) * 12 + (tmp8 & 0x0f); 
            }

            /* load instrument number */
            if (flags & (1<<6)) {
                h->read(&tmp8, 1, 1, h);
                rowsize--;
                if ((state->phase == 1) && (row_nr < state->module->patterns[pattern_nr].num_rows))
                    state->module->patterns[pattern_nr].rows[row_nr].data[channel_nr].sample_num = tmp8+1;
            }

            /* load volume */
            if (flags & (1<<5)) {
                h->read(&tmp8, 1, 1, h);
                rowsize--;
                if ((state->phase == 1) && (row_nr < state->module->patterns[pattern_nr].num_rows))
                    state->module->patterns[pattern_nr].rows[row_nr].data[channel_nr].volume = (tmp8 >> 1);
            }

            /* load effect */
            if (flags & (1<<4)) {
                h->read(&tmp8, 1, 1, h);
                h->read(&tmp8_2, 1, 1, h);
                rowsize-=2;

                if ((state->phase == 1) && (row_nr < state->module->patterns[pattern_nr].num_rows)) {
                    loader_psm_masi_translate_effect(&tmp8, &tmp8_2);
                    state->module->patterns[pattern_nr].rows[row_nr].data[channel_nr].effect_num = tmp8;
                    state->module->patterns[pattern_nr].rows[row_nr].data[channel_nr].effect_value = tmp8_2;
                }
                

            }
        }
        row_nr++;
        
       
    }
    
    return 0;
}