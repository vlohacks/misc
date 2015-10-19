/* 
 * File:   protracker.c
 * Author: vlo
 * 
 * Various module handling stuff
 *
 * Created on 29. Juni 2013, 13:57
 */

#include "module.h"
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
    
    for (i = 0; i < module->num_instruments; i++) {
        if (module->instruments[i].volume_envelope.nodes) 
            free(module->instruments[i].volume_envelope.nodes);

        if (module->instruments[i].pan_envelope.nodes) 
            free(module->instruments[i].pan_envelope.nodes);

        if (module->instruments[i].pitch_envelope.nodes)
            free(module->instruments[i].pitch_envelope.nodes);
    }
    
    if (module->instruments) {
        free(module->instruments);
        module->instruments = 0;
    }
    
    for (i = 0; i < module->num_patterns; i++) {
        if (module->patterns[i].rows) {
            for (j = 0; j < module->patterns[i].num_rows; j++) 
                free(module->patterns[i].rows[j].data);
        
            free(module->patterns[i].rows);
        }
    }
    free(module->patterns);
    module->patterns = 0;

    if (module->song_message)
        free(module->song_message);
    
    module->song_message = 0;

    return 0;
}

