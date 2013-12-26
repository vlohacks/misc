/* 
 * File:   protracker.c
 * Author: vlo
 * 
 * Various module handling stuff
 *
 * Created on 29. Juni 2013, 13:57
 */

#include "module.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

/* Free all mem occupied by module
 */
int module_free(module_t * module) 
{
    
    int i, j;
    
    for (i = 0; i < module->num_samples; i++) {
        if (module->samples[i].data)
                free(module->samples[i].data);
    }
    
    free(module->samples);
    module->samples = 0;
    
    for (i = 0; i < module->num_patterns; i++) {
        for (j = 0; j < module->patterns[i].num_rows; j++) 
            free(module->patterns[i].rows[j].data);

        free(module->patterns[i].rows);
    }
    free(module->patterns);
    module->patterns = 0;

    return 0;
}

/* Dump module data in human-readable form
 */
void module_dump(module_t * module, FILE * fd)
{
    int i, j, k;
    
    char note[4];
    char effect[2];
    char volume[3];
    
    if (fd == 0)    
        fd = stderr;
    
    for (i = 0; i < module->num_samples; i++) {
        fprintf(fd, "%02i %22s: size=%7u, fine=%2i, volume=%2u, loopstart=%7u, looplength=%7u\n", 
                i + 1, module->samples[i].header.name, module->samples[i].header.length, module->samples[i].header.finetune, module->samples[i].header.volume, module->samples[i].header.loop_start, module->samples[i].header.loop_length);
    }
    
    for (i = 0; i < module->num_patterns; i++) {
        fprintf(fd, "\npattern %i / %i:\n", i, module->num_patterns);
        for (j = 0; j < module->patterns[i].num_rows; j++) {
            fprintf(fd, "%02d | ", j);
            for (k = 0; k < module->num_channels; k++) {
                module_pattern_data_t * data = &(module->patterns[i].rows[j].data[k]);
                ui_periodindex2note(data->period_index, note);
                //sprintf(note, "%03i", data->period_index);
                ui_map_effect_num(effect, module->module_type, data->effect_num);
                if (data->volume > 64)
                    sprintf(volume, "%s", "..");
                else
                    sprintf(volume, "%02i", data->volume);
                
                fprintf(fd, "%s %02d %s %s%02X | ", note, data->sample_num, volume, effect, data->effect_value);
            }
            fprintf(fd, "\n");
        }
    }
}