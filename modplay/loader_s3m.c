#include "loader_s3m.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* loads a scream tracker 3 module (s3m)
 */
module_t * loader_s3m_loadfile(char * filename)
{
    char signature[4];
    int r, i, j, k;
    uint32_t tmp_u32;
    uint16_t tmp_u16;
    uint8_t tmp_u8;
    uint8_t channel_map[32];
    uint32_t sample_memseg;
    
    uint16_t num_patterns_internal;
    uint16_t * parapointer_pattern;
    uint16_t * parapointer_sample;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    FILE * f = fopen(filename, "rb");
    
    if (!module || !f) {
        if (f)
            fclose(f);
        if (module)
            free(module);
        return 0;
    }    
    
    module->module_type = module_type_s3m;

    /* chech if we really deal with a S3M file. IF not, bail out */
    fseek(f, 0x2c, SEEK_SET);
    r = fread(signature, 1, 4, f);
    if (memcmp(signature, "SCRM", 4)) {
        fclose(f);
        free(module);
        return 0;
    }
    
    /* read the song name */
    fseek(f, 0, SEEK_SET);
    r = fread(module->song_title, 1, 28, f);
    printf("%s\n", module->song_title);

    fseek(f, 0x20, SEEK_SET);    
    
    /* read num_orders, num_samples, num_patterns */
    r = fread(&(module->num_orders), sizeof(uint16_t), 1, f);
    r = fread(&(module->num_samples), sizeof(uint16_t), 1, f);
    r = fread(&num_patterns_internal, sizeof(uint16_t), 1, f);
    
    /* read flags */
    r = fread(&tmp_u16, sizeof(uint16_t), 1, f);
    module->module_info.flags_s3m.st2vibrato            = tmp_u16 & 1;
    module->module_info.flags_s3m.st2tempo              = tmp_u16 & 2;
    module->module_info.flags_s3m.amigaslides           = tmp_u16 & 4;
    // TODO left out "0vol optimizations"
    module->module_info.flags_s3m.amigalimits           = tmp_u16 & 16;
    // TODO left out "filter/sfx"
    module->module_info.flags_s3m.st30volumeslides      = tmp_u16 & 64;
    
    /* read version */
    r = fread(&tmp_u16, sizeof(uint16_t), 1, f);
    module->module_info.flags_s3m.st3_version           = tmp_u16;
    printf("Version: %04x\n", module->module_info.flags_s3m.st3_version);
    
    /* st3 3.0 suffers from bug where volume slides start also on tick 0.
     * Enable crippled volume slides to be compatible with files created with
     * this buggy version
     * This flag gets checked in effects */
    if (module->module_info.flags_s3m.st3_version == 0x1300)
        module->module_info.flags_s3m.st30volumeslides = 1;
        
    /* TODO: currently ignoring sample format, as it is always unsigned */
    
    /* read initial speed, tempo, master volume */
    fseek(f, 0x31, SEEK_SET);    
    r = fread(&(module->initial_speed), sizeof(uint8_t), 1, f);
    r = fread(&(module->initial_bpm), sizeof(uint8_t), 1, f);
    r = fread(&(module->initial_master_volume), sizeof(uint8_t), 1, f);
    fseek(f, 0x35, SEEK_SET); // skip ultraclick stuff... gus is dead.
    r = fread(&(module->module_info.flags_s3m.default_panning), sizeof(uint8_t), 1, f);
    
    printf("init speed: %i, init_bpm: %i, init_master_vol: %i\n", module->initial_speed, module->initial_bpm, module->initial_master_volume);
    
    /* TODO: we currently ignore GLOBAL_VOLUME .. maybe it will turn out that
     it is a good idea to deal with it... */
    
    /* initialize channel map */
    for (i = 0; i < 32; i++)
        channel_map[i] = 255;
    
    /* read channel infos */
    fseek(f, 0x40, SEEK_SET);
    module->num_channels = 0;
    for (i = 0; i < 32; i++) {
        
        r = fread(&tmp_u8, sizeof(uint8_t), 1, f);
        // channel enabled?
        if (tmp_u8 < 16) {
            channel_map[i] = module->num_channels;
            
            if (tmp_u8 <= 7) 
                module->initial_panning[module->num_channels] = 0;
            else 
                module->initial_panning[module->num_channels] = 0xff;
            
            module->num_channels ++;
        }
    }
    
    /* read orders and determine REAL number of patterns FS3MDOC.TXT says we
     * cannot rely on what the header data says */
    j = 0;
    module->num_patterns = 0;
    for (i = 0; i < module->num_orders; i++) {
        r = fread(&tmp_u8, sizeof(uint8_t), 1, f);
        if (tmp_u8 < 254) {
            module->orders[j++] = tmp_u8;
            if (tmp_u8 > module->num_patterns) 
                module->num_patterns = tmp_u8;
        }
    }
    module->num_orders = j;
    module->num_patterns++;
    
    printf ("num orders: %i, num_patterns: %i, num_channels: %i\n", module->num_orders, module->num_patterns, module->num_channels);
    for (i = 0; i < module->num_orders; i++)
        printf(" %i\n", module->orders[i]);
    
    /* read sample and patter parapointers */
    parapointer_sample = (uint16_t *)malloc(sizeof(uint16_t) * module->num_samples);
    parapointer_pattern = (uint16_t *)malloc(sizeof(uint16_t) * num_patterns_internal);
        
    fread(parapointer_sample, sizeof(uint16_t), module->num_samples, f);
    fread(parapointer_pattern, sizeof(uint16_t), num_patterns_internal, f);
    
    
    /* read default pan positions 
     * TODO: calculate the right panning values for 256 steps
     */
    if (module->module_info.flags_s3m.default_panning == 0xfc) {
        fread (module->initial_panning, sizeof(uint8_t), 32, f);
        for (i = 0; i < 32; i++) 
            module->initial_panning[i] = (module->initial_panning[i] << 4) | ((module->initial_panning[i] << 1) + (module->initial_panning[i]>6?1:0));
    }
    
    

    if ((module->initial_master_volume & 128) == 0) {
        /* make the song mono */
        for (i=0; i<32; i++) 
            module->initial_panning[i] = 0x7f;
        
        module->module_info.flags_s3m.mono = 1;
    } else {
        /* remove any garbage from the upper 4 bits */
        for (i=0; i<32; i++)
            module->initial_panning[i] &= 0x0f;     
       
        module->module_info.flags_s3m.mono = 0;
    }
    
    module->samples = malloc(sizeof(module_sample_t) * module->num_samples) ;
    /* read sample headers (instruments) */
    for (i = 0; i < module->num_samples; i++) {
        uint8_t sample_type;
        
        /* we use c2spd - initialize finetune with 0 */
        module->samples[i].header.finetune = 0;
        
        fseek(f, parapointer_sample[i] << 4, SEEK_SET);
        
        /* read sample type */
        fread(&sample_type, sizeof(uint8_t), 1, f);
        
        /* skip the "dos filename" 
         * FS3MDOC.TXT is wrong here, it states this are 13 chars,
         * actually it's 12 chars according to ST3 TECH.DOC
         */
        fseek(f, 12, SEEK_CUR);
        
        /* read sample "memseg" - which is stored in 3 bytes */
        fread(&tmp_u8, sizeof(uint8_t), 1, f);
        fread(&tmp_u16, sizeof(uint16_t), 1, f);
        sample_memseg = ((uint32_t)tmp_u8 << 16) + tmp_u16;
        
        /* sample length */
        fread(&tmp_u32, sizeof(uint32_t), 1, f);
        module->samples[i].header.length = tmp_u32 & 0xffff;
        
        /* loop start */
        fread(&tmp_u32, sizeof(uint32_t), 1, f);
        module->samples[i].header.loop_start = tmp_u32 & 0xffff;

        /* loop end */
        fread(&tmp_u32, sizeof(uint32_t), 1, f);
        module->samples[i].header.loop_end = tmp_u32 & 0xffff;
        module->samples[i].header.loop_length = module->samples[i].header.loop_end - module->samples[i].header.loop_start;
        
        /* volume */
        fread(&(module->samples[i].header.volume), sizeof(uint8_t), 1, f);
        
        /* Skip unused byte and packing scheme */
        fseek(f, 2, SEEK_CUR);
        
        /* flags 
         * (1)          = loop
         * (1<<1)       = stereo sample (never used, ignored for now)
         * (1<<2)       = 16 bit sample (never used, ignored for now)
         */
        fread(&tmp_u8, sizeof(uint8_t), 1, f);
        module->samples[i].header.loop_enabled = tmp_u8 & 1;    // loop flag
        
        /* c2spd */
        fread(&tmp_u32, sizeof(uint32_t), 1, f);
        module->samples[i].header.c2spd = tmp_u32 & 0xffff;

        /* Skip unused bytes */
        fseek(f, 12, SEEK_CUR);

        /* sample name */
        fread(module->samples[i].header.name, 1, 28, f);
        
        /* if we deal with a adlib instrument or a empty sample slot, continue
         * without loading data
         */
        if (sample_type != 1) {
            module->samples[i].data = 0;
            if (sample_type > 1)
                fprintf(stderr, __FILE__ " sample %i - unsupported type: %i (most likely ADLIB)\n", i ,sample_type);
            continue;
        }
        
        /* fetch sample data */
        module->samples[i].data = malloc(module->samples[i].header.length);
        fseek(f, sample_memseg << 4, SEEK_SET);
        fread(module->samples[i].data, 1, module->samples[i].header.length, f);
        
         /* we use unsigned samples interally */
        for (j=0; j<module->samples[i].header.length; j++)
            module->samples[i].data[j] ^= 128;
            
    }
    
    for (i=0; i< module->num_samples; i++) {
        module_sample_header_t * h = &(module->samples[i].header);
        printf ("l: %02i ls:%02i le:%02i v:%02i c2:%02i name:%s\n",
                h->length,
                h->loop_start,
                h->loop_end,
                h->volume,
                h->c2spd,
                h->name);
    }
        
    /* allocate patterns */
    module->patterns = (module_pattern_t *)malloc(sizeof(module_pattern_t) * num_patterns_internal);
   
    /* read pattern data */
    int pattern_nr = 0;
    uint8_t packed_flags;
    uint8_t channel_num;
    uint16_t packed_size;
    module_pattern_data_t tmp_data;
    
    for (i = 0; i < num_patterns_internal; i++) {
        fseek(f, parapointer_pattern[i] << 4, SEEK_SET);
        
        fread(&packed_size, sizeof(uint16_t), 1, f);
        
        module->patterns[pattern_nr].rows = (module_pattern_row_t *)malloc(sizeof(module_pattern_row_t) * 64);
        module->patterns[pattern_nr].num_rows = 64;
        
        for (j = 0; j < 64; j++) {
            module->patterns[pattern_nr].rows[j].data = (module_pattern_data_t *)malloc(sizeof(module_pattern_data_t) * module->num_channels);
            for (k = 0; k < module->num_channels; k++) {
                memset(&(module->patterns[pattern_nr].rows[j].data[k]), 0, sizeof(module_pattern_data_t));
                module->patterns[pattern_nr].rows[j].data[k].period_index = -1;
                module->patterns[pattern_nr].rows[j].data[k].volume = -1;
            }
        }
        
        for (j = 0; j < 64; j++) {
            
            do {
                memset(&tmp_data, 0, sizeof(module_pattern_data_t));
                tmp_data.period_index = -1;
                tmp_data.volume = -1;
                
                fread(&packed_flags, sizeof(uint8_t), 1, f);
                
                if (packed_flags > 0) {
                
                    channel_num = packed_flags & 31;

                    if (packed_flags & 32) {
                        fread(&(tmp_u8), 1, 1, f);
                        if (tmp_u8 == 255)
                            tmp_data.period_index = -1;
                        else if (tmp_u8 == 254)
                            tmp_data.period_index = 254;
                        else
                            tmp_data.period_index = (int)(((tmp_u8 >> 4) * 12) + (tmp_u8 & 0x0f));

                        fread(&(tmp_data.sample_num), 1, 1, f);
                    }

                    if (packed_flags & 64)
                        fread(&tmp_data.volume, 1, 1, f);

                    if (packed_flags & 128) {
                        fread(&tmp_data.effect_num, 1, 1, f);
                        fread(&tmp_data.effect_value, 1, 1, f);
                    }
                    
                    if (channel_map[channel_num] < 255) {
                        //printf("r=%i, cn=%i, ch=%i\n", j, channel_num, channel_map[channel_num]);
                        memcpy(&(module->patterns[pattern_nr].rows[j].data[channel_map[channel_num]]), &tmp_data, sizeof(module_pattern_data_t));
                    }                    
                }
                

                
            } while (packed_flags);
        }
        pattern_nr++;
    }
        
    
    /* free memory temporary occupied by parapointers */
    free (parapointer_sample);
    free (parapointer_pattern);
    
    //module_dump(module, stdout);
    
    return module;
}

