/* 
 * File:   loader_mod.c
 * Author: vlo
 * 
 * Loader routines for .mod files
 *
 * Created on 29. Juni 2013, 13:57
 */
#include <stdlib.h>
#include <string.h>

#include "loader_mod.h"
#include "defs_mod.h"
#include "arch.h"
#include "io.h"

/* checks if given data is a mod, returns 1 if data is valid */
int loader_mod_check(io_handle_t * h)
{
    int r, i;
    uint32_t signature = 0;
    size_t saved_pos;
    char tmp[5];
    
    saved_pos = h->tell(h);
    h->seek(h, 0x438, SEEK_SET);
    r = h->read(&signature, 1, 4, h);
    h->seek(h, saved_pos, io_seek_set);
    
    if (r != 4) 
        return 0;

    // Probe for standard MOD types
    for (i = 0; i < loader_mod_num_modtypes; i++) {
        if (signature == loader_mod_modtypes[i].signature) 
            return 1;
    }
    
    // Probe for FT2 xxCH 10 channel+ signature
    for (i = 10; i <= 32; i += 2) {
        sprintf(tmp, "%02iCH", i);
        if (memcmp (&signature, tmp, 4) == 0) 
            return 1;
    }
    
    /* TODO: Check if the file MIGHT BE a STK (get sample sizes, compare to file
     * size since there is no magic/header
     */
        
    return 0;
}

/* Loads a protracker/startrekker/soundtracker module file (*.mod, *.stk)
 */
module_t * loader_mod_load(io_handle_t * h)
{
    int i, j, k, r;
    uint8_t tmp8;
    char tmp[5];
    
    uint32_t signature;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    
    if (!module)
        return 0;
    
    // for all mods the effects and data format is the same, so the file type
    // is mod, regardless of being a stm or multichannel MOD file
    module->module_type = module_type_mod;
    
    // Default values for MOD files
    module->initial_bpm = 125;
    module->initial_speed = 6;
    
    // Determine mod file type by checking the signatuer (M.K., nCHN...)
    h->seek(h, 0x438, SEEK_SET);
    r = h->read(&signature, 1, 4, h);
    if (r != 4) {
        free(module);
        return 0;
    }

    // Probe for standard MOD types
    module->num_samples = 0;
    for (i = 0; i < loader_mod_num_modtypes; i++) {
        if (signature == loader_mod_modtypes[i].signature) {
            // valid signature means protracker mod with 31 sample slots
            module->num_channels = loader_mod_modtypes[i].num_channels;
            module->num_samples = 31;
            memcpy(module->module_info.flags_mod.signature, &signature, 4);
            module->module_info.flags_mod.signature[4] = 0;
            strcpy(module->module_info.description, loader_mod_modtypes[i].description);
            strcpy(module->module_info.default_file_extension, "MOD");
            break;
        }
    }
    
    // Probe for FT2 xxCH 10 channel+ signature
    for (i = 10; i <= 32; i += 2) {
        sprintf(tmp, "%02iCH", i);
        if (memcmp (&signature, tmp, 4) == 0) {
            module->num_channels = i;
            module->num_samples = 31;
            memcpy(module->module_info.flags_mod.signature, &signature, 4);
            module->module_info.flags_mod.signature[4] = 0;
            strcpy(module->module_info.description, "Fasttracker Multichannel MOD");
            strcpy(module->module_info.default_file_extension, "MOD");
            break;
        }
    }
    
    // no valid signature found means old SoundTracker MOD (STK) with 15 sample
    // slots and 4 channels
    if (module->num_samples == 0) {
        module->num_samples = 15;
        module->num_channels = 4;
        strcpy(module->module_info.default_file_extension, "STK");
        strcpy(module->module_info.flags_mod.signature, "(none)");
        strcpy(module->module_info.description, "Amiga Ultimate Soundtracker module");
    }

    // now we know what kind of MOD it is, so we can start with REAL loading
    h->seek(h, 0, io_seek_set);
    r = h->read(module->song_title, 1, 20, h);
    if (r != 20) {
        free(module);
        return 0;
    }

    // load sample header data (aka song message :-))
    module->samples = (module_sample_t *)malloc(sizeof(module_sample_t) * module->num_samples);
    for (i = 0; i < module->num_samples; i++) {
        if (loader_mod_read_sample_header(&(module->samples[i].header), h)) {
            free(module);
            return 0;
        }
    }

    // read number of orders in mod
    r = h->read(&tmp8, 1, 1, h);
    if (r != 1) {
        free(module);
        return 0;
    }

    module->num_orders = (uint16_t)tmp8;
    
    // read not used "load patterns" / "loop position" / whatever
    r = h->read(&tmp8, 1, 1, h);
    if (r != 1) {
        free(module);
        return 0;
    }

    // read order list
    r = h->read(&(module->orders), 1, 128, h);
    if (r != 128) {
        free(module);
        return 0;
    }
    
    // read signature again, just to move the filepointer - and only if the
    // file is not a STK not having a signature
    if (module->num_samples > 15) {
        r = h->read(&signature, 4, 1, h);
        if (r != 1) {
            free(module);
            return 0;
        }
    }
    
    // determine number of patterns im MOD by fetching highest entry in orders
    module->num_patterns = 0;
    for (i = 0;i < module->num_orders; i++) {
        if (module->orders[i] > module->num_patterns)
            module->num_patterns = module->orders[i];
    }
    module->num_patterns++;

    // load pattern data - TODO: special FLT8 arrangement - currently broken
    module->patterns = (module_pattern_t *)malloc(sizeof(module_pattern_t) * module->num_patterns);
    for (i = 0; i < module->num_patterns; i++) {
        module->patterns[i].num_rows = 64;        // mod alwas has 64 rows per pattern
        module->patterns[i].rows = (module_pattern_row_t *)malloc(sizeof(module_pattern_row_t) * module->patterns[i].num_rows);
        for (j = 0; j < module->patterns[i].num_rows; j++) {
            module->patterns[i].rows[j].data = (module_pattern_data_t *)malloc(sizeof(module_pattern_data_t) * module->num_channels);
            for (k = 0; k < module->num_channels; k++) {
                if(loader_mod_read_pattern_data (&(module->patterns[i].rows[j].data[k]), h)) {
                    free(module);
                    return 0;
                }
            }
        }
    }

    // load sample pcm data
    for (i = 0; i < module->num_samples; i++) 
        loader_mod_read_sample_data(&(module->samples[i]), h);
	
    // initial pannings 
    for (i = 0; i < module->num_channels; i++) {
        if (((i % 4) == 1) || ((i % 4) == 2))           // RLLR RLLR ...
            module->initial_panning[i] = 0x00;
        else
            module->initial_panning[i] = 0xff;
    }
    module->song_message = 0;
    return module;
}


int loader_mod_read_sample_header(module_sample_header_t * hdr, io_handle_t * h) 
{
    uint16_t word;
    uint8_t byte;
    int8_t sbyte;
    int r;

    r = h->read (&(hdr->name), 1, 22, h);
    if (r != 22)
        return 1;
    hdr->name[22] = 0;

    r = h->read (&word, 2, 1, h);
    if (r != 1)
        return 1;
    hdr->length = swap_endian_u16(word) << 1;

    r = h->read (&sbyte, 1, 1, h);
    if (r != 1)
        return 1;
    hdr->finetune = sbyte & 0xf; // > 7 ? -(16-sbyte) : sbyte;

    r = h->read (&byte, 1, 1, h);
    if (r != 1)
        return 1;
    hdr->volume = byte;
    
    r = h->read (&word, 2, 1, h);
    if (r != 1)
        return 1;
    hdr->loop_start = (swap_endian_u16(word) << 1);
    
    r = h->read (&word, 2, 1, h);
    if (r != 1)
        return 1;
    
    hdr->loop_length = (swap_endian_u16(word) << 1);
        
    hdr->loop_end = (hdr->loop_length + hdr->loop_start) - 1;
    
    hdr->loop_enabled = 0;
    if (hdr->loop_length > 2)
        hdr->loop_enabled = 1;
    
    return 0;
}

int loader_mod_read_pattern_data(module_pattern_data_t * data, io_handle_t * h) 
{
    uint32_t dw;
    uint16_t tmp;
    int r;

    r = h->read(&dw, 4, 1, h);
    if (r != 1)
        return 1;

    dw = swap_endian_u32(dw);

    // AWFUL Amiga SHIT (piss doch drauf... scheiß doch rein... zünd ihn an...)
    data->sample_num = ((uint8_t)(dw >> 24) & 0xf0) | ((uint8_t)(dw >> 12) & 0x0f);
    tmp = (uint16_t)((dw >> 16) & 0x0fff);
    data->period_index = loader_mod_lookup_period_index(tmp);
    
    if (tmp && data->period_index == -1) {
        fprintf(stderr, "Loader: WARNING: Non-standard period: %i @ %x\n", tmp, (unsigned int)h->tell(h));
    }
    
    data->effect_num = (uint8_t)((dw & 0x0f00) >> 8);
    data->effect_value = (uint8_t)(dw & 0xff);
    data->volume = -1;  // mod has no volume bar
    
    return 0;
}

void loader_mod_read_sample_data(module_sample_t * sample, io_handle_t * h)
{
    int i;
    int8_t * p;

    if (sample->header.length == 0) {
        sample->data = NULL;
        return;
    }

    sample->data = (int8_t *)malloc(sizeof(int8_t) * sample->header.length); //sample->header.length);
    p = sample->data;
    for (i = 0; i < sample->header.length; i++)
        h->read(p++, 1, 1, h);
}

int loader_mod_lookup_period_index(const uint16_t period)
{
    int i;
    for (i = 0; i < defs_mod_num_periods; i++) {
        if (defs_mod_periods[i] == period)
            return i;
    }
    return -1;
}