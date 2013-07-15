/* 
 * File:   loader_mod.c
 * Author: vlo
 * 
 * Loader routines for .mod files
 *
 * Created on 29. Juni 2013, 13:57
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "loader_mod.h"
#include "protracker.h"
#include "arch.h"

/* Loads a protracker/startrekker/soundtracker module file (*.mod, *.stk)
 */
module_t * loader_mod_loadfile(char * filename)
{
    int i, j, k, r;
    uint8_t tmp8;
    char tmp[5];
    
    uint32_t signature;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    FILE * f = fopen(filename, "rb");
    
    if (!module || !f) {
        if (f)
            fclose(f);
        if (module)
            free(module);
        return 0;
    }
    
    // for all mods the effects and data format is the same, so the file type
    // is mod, regardless of being a stm or multichannel MOD file
    module->module_type = module_type_mod;
    
    // Determine mod file type by checking the signatuer (M.K., nCHN...)
    fseek(f, 0x438, SEEK_SET);
    r = fread(&signature, 1, 4, f);
    if (r != 4) {
        free(module);
        fclose(f);
        return 0;
    }

    // Probe for standard MOD types
    module->num_samples = 0;
    for (i = 0; i < loader_mod_num_modtypes; i++) {
        if (signature == loader_mod_modtypes[i].signature) {
            // valid signature means protracker mod with 31 sample slots
            module->num_channels = loader_mod_modtypes[i].num_channels;
            module->num_samples = 31;
            memcpy(module->module_info.signature, &signature, 4);
            module->module_info.signature[4] = 0;
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
            memcpy(module->module_info.signature, &signature, 4);
            module->module_info.signature[4] = 0;
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
        strcpy(module->module_info.signature, "(none)");
        strcpy(module->module_info.description, "Amiga Ultimate Soundtracker module");
    }

    // now we know what kind of MOD it is, so we can start with REAL loading
    fseek(f, 0, SEEK_SET);
    r = fread(module->song_title, 1, 20, f);
    if (r != 20) {
        free(module);
        fclose(f);
        return 0;
    }

    // load sample header data (aka song message :-))
    module->samples = (module_sample_t *)malloc(sizeof(module_sample_t) * module->num_samples);
    for (i = 0; i < module->num_samples; i++) {
        if (loader_mod_read_sample_header(&(module->samples[i].header), f)) {
            free(module);
            fclose(f);
            return 0;
        }
    }

    // read number of orders in mod
    r = fread(&tmp8, 1, 1, f);
    if (r != 1) {
        free(module);
        fclose(f);
        return 0;
    }

    module->num_orders = (uint16_t)tmp8;
    
    // read not used "load patterns" / "loop position" / whatever
    r = fread(&tmp8, 1, 1, f);
    if (r != 1) {
        free(module);
        fclose(f);
        return 0;
    }

    // read order list
    r = fread(&(module->orders), 1, 128, f);
    if (r != 128) {
        free(module);
        fclose(f);
        return 0;
    }
    
    // read signature again, just to move the filepointer - and only if the
    // file is not a STK not having a signature
    if (module->num_samples > 15) {
        r = fread(&signature, 4, 1, f);
        if (r != 1) {
            free(module);
            fclose(f);
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
                if(loader_mod_read_pattern_data (&(module->patterns[i].rows[j].data[k]), f)) {
                    if (module->patterns[i].rows[j].data[k].period_index == -1) {
                        fprintf(stderr, "Loader: WARNING: Non-standard period in pattern: %i, channel: %i, row: %i\n", i, j, k);
                    }                    
                    free(module);
                    fclose(f);
                    return 0;
                }
            }
        }
    }

    // load sample pcm data
    for (i = 0; i < module->num_samples; i++) 
        loader_mod_read_sample_data(&(module->samples[i]), f);
	
    fclose(f);
    
    return module;
}


int loader_mod_read_sample_header(module_sample_header_t * hdr, FILE * f) 
{
    uint16_t word;
    uint8_t byte;
    int8_t sbyte;
    int r;

    r = fread (&(hdr->name), 1, 22, f);
    if (r != 22)
        return 1;
    hdr->name[22] = 0;

    r = fread (&word, 2, 1, f);
    if (r != 1)
        return 1;
    hdr->length = swap_endian_u16(word) << 1;

    r = fread (&sbyte, 1, 1, f);
    if (r != 1)
        return 1;
    hdr->finetune = sbyte & 0xf; // > 7 ? -(16-sbyte) : sbyte;

    r = fread (&byte, 1, 1, f);
    if (r != 1)
        return 1;
    hdr->volume = byte;
    
    r = fread (&word, 2, 1, f);
    if (r != 1)
        return 1;
    hdr->loop_start = swap_endian_u16(word) << 1;
    
    r = fread (&word, 2, 1, f);
    if (r != 1)
        return 1;
    hdr->loop_length = swap_endian_u16(word) << 1;
    
    return 0;
}

int loader_mod_read_pattern_data(module_pattern_data_t * data, FILE * f) 
{
    uint32_t dw;
    int r;

    r = fread(&dw, 4, 1, f);
    if (r != 1)
        return 1;

    dw = swap_endian_u32(dw);

    // AWFUL Amiga SHIT (piss doch drauf... scheiß doch rein... zünd ihn an...)
    data->sample_num = ((uint8_t)(dw >> 24) & 0xf0) | ((uint8_t)(dw >> 12) & 0x0f);
    data->period = (uint16_t)((dw >> 16) & 0x0fff);
    data->period_index = protracker_lookup_period_index(data->period);
    data->effect_num = (uint8_t)((dw & 0x0f00) >> 8);
    data->effect_value = (uint8_t)(dw & 0xff);
    
    return 0;
}

void loader_mod_read_sample_data(module_sample_t * sample, FILE * f)
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
        *p++ = fgetc(f);
}