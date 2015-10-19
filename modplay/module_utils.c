#include "module_utils.h"
#include <stdio.h>
#include "murmur2.h"
#include "dllist.h"

/* Dumps a internal module structure back to a constant C structure
 * (for embedded dev to store a loaded mod in Flash memory)
 */
void module_dump_c(module_t * module) 
{
    int i, j, k;
    uint32_t hash;
    dllist_t list;
    dllist_element_t * tmp_elem;
    
    list.first=list.last=list.count = 0;
    
    printf("#ifndef _CONSTMOD_H\n#define _CONSTMOD_H\n\n#include \"module.h\"\n");
            
    
    for (i = 0; i < module->num_samples; i++) {
        printf ("const int8_t sample_data_%02i[] = {", i);
        for (j = 0; j < module->samples[i].header.length; j++) {
            if (!(j % 16))
                printf("\n");
            printf("%6i", sample_to_s8(module->samples[i].data[j]));
            if (j+1 < module->samples[i].header.length)
                printf(", ");
        }
        printf("};\n\n");
    }
    
    printf ("const module_sample_t samples[] = { ");
    for (i = 0; i < module->num_samples; i++) {
        printf("\t{ \n");
        printf("\t\t.header.length = %u,\n", module->samples[i].header.length);
        printf("\t\t.header.finetune = %i,\n", module->samples[i].header.finetune);
        printf("\t\t.header.c2spd = %u,\n", module->samples[i].header.c2spd);
        printf("\t\t.header.volume = %u,\n", module->samples[i].header.volume);
        printf("\t\t.header.loop_enabled = %u,\n", module->samples[i].header.loop_enabled);
        printf("\t\t.header.loop_start = %u,\n", module->samples[i].header.loop_start);
        printf("\t\t.header.loop_length = %u,\n", module->samples[i].header.loop_length);
        printf("\t\t.header.loop_end = %u,\n", module->samples[i].header.loop_end);
        // left out the XM/IT stuff here
        printf("\t\t.data = sample_data_%02i\n", i);
        printf("\t},\n");
    }
    printf("};\n");
    
    for (i = 0; i < module->num_patterns; i++) {
        for (j = 0; j < module->patterns[i].num_rows; j++) {
            /* Duplicated rows are avoided by hashing the row data 
             * Flash memory is rare ... this was the only way getting
             * GLOW_BUG.S3M into the flash of an STM32F104 along with 
             * the player ;-)
             */
            hash = MurmurHash2A(module->patterns[i].rows[j].data, (sizeof(module_pattern_data_t) * module->num_channels), 0xdeadbeef);
            /* check if this row already exists */
            for (tmp_elem = list.first; tmp_elem; tmp_elem = tmp_elem->next) {
                if (!tmp_elem)
                    break;
                if ((uint32_t)(tmp_elem->data) == (void *)hash)
                    break;
            }
            /* only if the row does not exist, add it's hash to the list and output data */
            if (!tmp_elem) {
                tmp_elem = malloc(sizeof(dllist_element_t));
                tmp_elem->next = tmp_elem->prev = 0; // init
                tmp_elem->data = hash;
                dllist_push(&list, tmp_elem);

                printf("const module_pattern_data_t pattern_data_%08x[] = {", hash);
                for (k = 0; k < module->num_channels; k++) {
                    module_pattern_data_t * data = &(module->patterns[i].rows[j].data[k]);
                    printf("{.sample_num=%3u, .period_index=%6i, .volume=%4i, .effect_num=%3u, .effect_value=%3u }", data->sample_num, data->period_index, data->volume, data->effect_num, data->effect_value);
                    if (k+1 < module->num_channels)
                        printf(", ");
                }
                printf("};\n");
            }
        }
        printf("const module_pattern_row_t pattern_rows_%02i[] = {\n", i);
        for (j = 0; j < module->patterns[i].num_rows; j++) {
            hash = MurmurHash2A(module->patterns[i].rows[j].data, (sizeof(module_pattern_data_t) * module->num_channels), 0xdeadbeef);
            printf("{ .data = pattern_data_%08x }", hash);
            if (j+1 < module->patterns[i].num_rows)
                printf(", ");
            
        }
        printf("\n};\n\n");
    }
    
    printf("const module_pattern_t patterns[] = {\n");
    for (i = 0; i < module->num_patterns; i++) {
        printf("{ .num_rows = %03u, .rows = pattern_rows_%02i }", module->patterns[i].num_rows, i);
        if (i+1 < module->num_patterns)
            printf(", ");
        
    }
    printf("};\n");
    
    printf("const module_t static_module = {\n");
    printf("\t.song_title = { ");
    for (i = 0; i < 21; i++) {
        printf(" %3i", module->song_title[i]);
        if (i < 20)
            printf(", ");
    }
    printf("},\n");
    printf("\t.module_type = %i,\n", module->module_type);
    printf("\t.num_channels = %u,\n", module->num_channels);
    printf("\t.num_samples = %u,\n", module->num_samples);
    printf("\t.num_instruments = %u,\n", module->num_instruments);
    printf("\t.num_patterns = %u,\n", module->num_patterns);
    printf("\t.num_orders = %u,\n", module->num_orders);
    printf("\t.orders = { ");
    for (i = 0; i < 256; i++) {
        printf(" %3u", module->orders[i]);
        if (i < 255)
            printf(", ");
    }
    printf("},\n");
    printf("\t.samples = samples,\n");
    printf("\t.patterns = patterns,\n");
    printf("\t.initial_speed = %u,\n", module->initial_speed);
    printf("\t.initial_bpm = %u,\n", module->initial_bpm);
    printf("\t.initial_master_volume = %u,\n", module->initial_master_volume);
    printf("\t.mix_volume = %u,\n", module->mix_volume);
    printf("\t.initial_panning = { ");
    for (i = 0; i < 64; i++) {
        printf(" %3u", module->initial_panning[i]);
        if (i < 63)
            printf(", ");
    }
    printf("},\n");    
    printf("\t.channel_volume = { ");
    for (i = 0; i < 64; i++) {
        printf(" %3u", module->channel_volume[i]);
        if (i < 63)
            printf(", ");
    }
    printf("}\n");    

    
    printf("};\n");
    
    
    printf("#endif");
    
    while ((tmp_elem = dllist_pop(&list)) != 0)
        free(tmp_elem);
    
}



/* Dump module data in human-readable form
 */
void module_dump(module_t * module, FILE * fd)
{
    int i, j, k;
    uint32_t hash;
    
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
        fprintf(fd, "====%i====\n", module->patterns[i].num_rows);
        for (j = 0; j < module->patterns[i].num_rows; j++) {
            fprintf(fd, "%02d | ", j);
            for (k = 0; k < module->num_channels; k++) {
                module_pattern_data_t * data = &(module->patterns[i].rows[j].data[k]);
                ui_periodindex2note(data->period_index, note, module->module_type == module_type_it ? 0 : 1);
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
