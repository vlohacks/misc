#include "loader_mtm.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mixing.h"

/* checks if given data is a MTM, returns 1 if data is valid 
 */
int loader_mtm_check(io_handle_t * h)
{
    char signature[3];
    size_t saved_pos;
    
    memset(signature, 0, 3);
    
    saved_pos = h->tell(h);
    h->seek(h, 0, io_seek_set);
    h->read(signature, 1, 3, h);
    h->seek(h, saved_pos, io_seek_set);
        
    if (memcmp(signature, "MTM", 3))
        return 0;

    return 1;
}


module_t * loader_mtm_load(io_handle_t * h)
{

    int i, j, k;
    uint8_t tmp_u8;
    uint16_t num_tracks;
    uint16_t song_message_length;
    
    typedef uint8_t track_data_item_t[3];
    typedef track_data_item_t track_data_t[64];

    track_data_t * track_data;
    uint8_t num_channels;
    
    if (!loader_mtm_check(h))
        return 0;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    
    if (!module) 
        return 0;
    
    module->initial_bpm = 125;
    module->initial_speed = 6;
    module->module_type = module_type_mtm;
    // has no instruments..
    module->instruments = 0;
    module->num_instruments = 0;
    
    h->seek(h, 3, io_seek_set);
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    
    h->read(&(module->song_title), sizeof(char), 20, h);
    h->read(&num_tracks, sizeof(uint16_t), 1, h);
    
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    module->num_patterns = ((uint16_t)tmp_u8) + 1;
    
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    module->num_orders = ((uint16_t)tmp_u8) + 1;
    
    h->read(&song_message_length, sizeof(uint16_t), 1, h);
    
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    module->num_samples = ((uint16_t)tmp_u8);

    /* attribute byte (unused) */
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    
    /* beats per track TODO: what's this?? */
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);
    
    
    h->read(&num_channels, sizeof(uint8_t), 1, h);
    module->num_channels = num_channels;
    
    for (i = 0; i < 32; i++) {
        h->read(&tmp_u8, sizeof(uint8_t), 1, h);
        module->initial_panning[i] = tmp_u8 << 4;
    }
    
    module->samples = (module_sample_t *)malloc(sizeof(module_sample_t) * module->num_samples);
    
    for (i = 0; i < module->num_samples; i++) {
        h->read(module->samples[i].header.name, sizeof(char), 22, h);
        h->read(&(module->samples[i].header.length), sizeof(uint32_t), 1, h);
        h->read(&(module->samples[i].header.loop_start), sizeof(uint32_t), 1, h);
        h->read(&(module->samples[i].header.loop_end), sizeof(uint32_t), 1, h);
        module->samples[i].header.loop_end -= 1;
        module->samples[i].header.loop_length = (module->samples[i].header.loop_end - module->samples[i].header.loop_start) + 1;
        if (module->samples[i].header.loop_length > 2)
            module->samples[i].header.loop_enabled = 1;
        else
            module->samples[i].header.loop_enabled = 0;
        
        h->read(&(module->samples[i].header.finetune), sizeof(uint8_t), 1, h);
        h->read(&(module->samples[i].header.volume), sizeof(uint8_t), 1, h);
        h->read(&tmp_u8, sizeof(uint8_t), 1, h);
        
        if (tmp_u8 & 1) {
            fprintf(stderr, __FILE__ " Sample #%i is 16 Bit und currently unsupported\n", i);
            continue;
        }
        
    }
    
    /* read order data */
    for (i = 0; i < 128; i++) {
        h->read(&tmp_u8, sizeof(uint8_t), 1, h);
        module->orders[i] = tmp_u8;
    }
    
    /* read track data, which is stored in MTM independent of pattern data 
     ŧhe track data is 1 based, track 0 is always blank */
    track_data = (track_data_t *)calloc(sizeof(track_data_t), num_tracks + 1);
    for (i = 1; i <= num_tracks; i++) 
        h->read(track_data[i], sizeof(track_data_t), 1, h);

        
    /* read pattern sequencing data referring to the track data read above */
    uint16_t track_num;
    module->patterns = (module_pattern_t *)malloc(sizeof(module_pattern_t) * module->num_patterns);
    for (i = 0; i < module->num_patterns; i++) {
        module->patterns[i].num_rows = 64;
        module->patterns[i].rows = (module_pattern_row_t *)malloc(sizeof(module_pattern_row_t) * 64);
        for (j = 0; j < 64; j++) {
            module->patterns[i].rows[j].data = (module_pattern_data_t *) malloc (sizeof(module_pattern_data_t) * module->num_channels);
        }
        for (j = 0; j < 32; j++) {
            h->read(&track_num, sizeof(uint16_t), 1, h);
            if (j < module->num_channels) {
                for (k = 0; k < 64; k++) {
                    
                    tmp_u8 = track_data[track_num][k] [0] >> 2;
                    
                    //tmp_u8 = track_data[track_num][k].period_index;
                    
                    if (tmp_u8 == 0) {
                        module->patterns[i].rows[k].data[j].period_index = -1;
                    } else {
                        module->patterns[i].rows[k].data[j].period_index = tmp_u8; //(int)(((tmp_u8 >> 4) * 12) + (tmp_u8 & 0x0f));
                    }
                    /*
                    module->patterns[i].rows[k].data[j].sample_num = track_data[track_num][k].sample_num;  //((track_data[track_num][k][0] & 0b11) << 4) | ((track_data[track_num][k][1] & 0b11110000) >> 4);
                    module->patterns[i].rows[k].data[j].effect_num = track_data[track_num][k].effect_num; //  [1] & 0x0f;
                    module->patterns[i].rows[k].data[j].effect_value = track_data[track_num][k].effect_value; //[2];
                    */
                    
                    module->patterns[i].rows[k].data[j].sample_num = ((track_data[track_num][k][0] & 0b11) << 4) | ((track_data[track_num][k][1] & 0b11110000) >> 4);
                    module->patterns[i].rows[k].data[j].effect_num = track_data[track_num][k][1] & 0x0f;
                    module->patterns[i].rows[k].data[j].effect_value = track_data[track_num][k][2];
                    
                    module->patterns[i].rows[k].data[j].volume = -1;
                     
                }
            }
        }
        
    }
    
    /* song message */
    if (song_message_length) {
        module->song_message = (char *)malloc(song_message_length);
        h->read(module->song_message, 1, song_message_length, h);
    }
    
    
    /* sample data */
    for (i = 0; i < module->num_samples; i++) {
        module_sample_header_t * sh = &(module->samples[i].header);
        if (sh->length) {
            module->samples[i].data = (sample_t *)malloc(sizeof(sample_t) * sh->length);
            for (j = 0; j < sh->length; j++) {
                h->read(&tmp_u8, 1, 1, h);
                tmp_u8 ^= 128;
                module->samples[i].data[j] = sample_from_s8((int8_t)tmp_u8);
            }
        } else {
            module->samples[i].data = 0;
        }
        
    }
    
    
    //module_dump(module, stdout);
    
    free(track_data);
   
    return module;
}

