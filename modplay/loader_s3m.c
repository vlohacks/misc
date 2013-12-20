#include "loader_s3m.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* loads a scream tracker 3 module (s3m)
 */
module_t * loader_s3m_loadfile(char * filename)
{
    char signature[4];
    int r, i, j;
    uint16_t tmp_u16;
    uint8_t tmp_u8;
    uint8_t channel_map[32];
    
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

    /* chech if we really deal with a S3M file */
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
    r = fread(&(module->num_patterns), sizeof(uint16_t), 1, f);
    
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
    
    return module;
}

