#include "ui.h"
#include "module.h"
#include <string.h>
#include <stdio.h>

#include "defs_mod.h"
#include "defs_s3m.h"

void ui_periodindex2note(int period_index, char * dest, int first_octave)
{
    
    static const char * notes[] = {
        "C-",
        "C#",
        "D-",
        "D#",
        "E-",
        "F-",
        "F#",
        "G-",
        "G#",
        "A-",
        "A#",
        "B-"
    };
    
    int i;
    for (i=0; i<3; i++)
        dest[i] = '.';

    dest[3] = 0;

    if (period_index == 254) {
        strcpy(dest, "^^^");
        return;
    }
    
    //i = protracker_lookup_period_index(period);
    if (period_index >= 0)
        sprintf(dest, "%s%1u", notes[period_index % 12], (period_index / 12) + first_octave);
    
}



void ui_effect_to_humanreadable(char * buf, const uint8_t effect_num, const uint8_t * effect_values, const module_type_t module_type)
{
    
    
    *buf = 0;
    char effect[2];
    uint8_t effect_val = effect_values[effect_num];
    
    switch (module_type) {
        case module_type_mtm:
        case module_type_mod:
            switch (effect_num) {
                case 0: 
                    if (effect_val)
                        strcpy(buf, "arpeggio");
                    break;
                case 0x1: strcpy(buf, "portamento up"); break;
                case 0x2: strcpy(buf, "portamento down"); break;
                case 0x3: strcpy(buf, "portamento to note"); break;
                case 0x4: strcpy(buf, "vibrato"); break;
                case 0x5: strcpy(buf, "port. to note + vol. slide"); break;
                case 0x6: strcpy(buf, "vibrato + vol. slide"); break;
                case 0x7: strcpy(buf, "tremolo"); break;
                case 0x8: strcpy(buf, "panning"); break;
                case 0x9: strcpy(buf, "sample offset"); break;
                case 0xa: strcpy(buf, "volume slide"); break;
                case 0xb: strcpy(buf, "position jump"); break;
                case 0xc: strcpy(buf, "set volume"); break;
                case 0xd: strcpy(buf, "pattern break"); break;
                case 0xe: 
                    switch (effect_val >> 4) {
                        case 0x1: strcpy(buf, "fine porta up"); break;
                        case 0x2: strcpy(buf, "fine porta down"); break;
                        case 0x4: strcpy(buf, "set vibrato waveform"); break;
                        case 0x6: strcpy(buf, "pattern loop"); break;
                        case 0x7: strcpy(buf, "set tremolo waveform"); break;
                        case 0x8: strcpy(buf, "panning"); break;
                        case 0x9: strcpy(buf, "retrigger sample"); break;
                        case 0xa: strcpy(buf, "fine volume up"); break;
                        case 0xb: strcpy(buf, "fine volume down"); break;
                        case 0xc: strcpy(buf, "note cut"); break;
                        case 0xd: strcpy(buf, "note delay"); break;
                        case 0xe: strcpy(buf, "pattern delay"); break;
                        default: sprintf(buf, "UNIMPLEMENTED: %1x%2x", effect_num, effect_val); break;
                    }
                    break;
                case 0xf: strcpy(buf, "set speed"); break;            
            }
            break;
            
        case module_type_stm:
        case module_type_s3m:
            switch (effect_num) {
                case 0: break;
                case 1: strcpy(buf, "set speed"); break;
                case 3: strcpy(buf, "pattern break"); break;
                
                case 4: 
                    if ((effect_val & 0x0f) == 0x0f)
                        strcpy(buf, "fine volslide up"); 
                    else if ((effect_val & 0xf0) == 0xf0)
                        strcpy(buf, "fine volslide down"); 
                    
                    if ((effect_val & 0x0f) == 0x00)
                        strcpy(buf, "volume slide up"); 
                    else if ((effect_val & 0xf0) == 0x00)
                        strcpy(buf, "volume slide down"); 
                    
                    break;
                    
                case 5:
                    if ((effect_val & 0xf0) == 0xf0)
                        strcpy(buf, "fine portamento down"); 
                    else if ((effect_val & 0xf0) == 0xe0)
                        strcpy(buf, "extrafine portamento down"); 
                    else 
                        strcpy(buf, "portamento down"); 
                    break;                    

                case 6:
                    if ((effect_val & 0xf0) == 0xf0)
                        strcpy(buf, "fine portamento up"); 
                    else if ((effect_val & 0xf0) == 0xe0)
                        strcpy(buf, "extrafine portamento up"); 
                    else 
                        strcpy(buf, "portamento up"); 
                    break;                    

                case 7: strcpy(buf, "portamento to note"); break;
                case 8: strcpy(buf, "vibrato"); break;
                case 10: strcpy(buf, "arpeggio"); break;
                case 11: strcpy(buf, "vibrato + volume slide"); break;
                case 12: strcpy(buf, "note portamento + vol slide"); break;
                case 15: strcpy(buf, "sample offset"); break;
                case 17: strcpy(buf, "retrigger + volume slide"); break;
                case 18: strcpy(buf, "tremolo"); break;
                case 19: 
                    switch (effect_val >> 4) {
                        case 0x3: strcpy(buf, "set vibrato waveform"); break;
                        case 0x4: strcpy(buf, "set tremolo waveform"); break;
                        case 0x8: strcpy(buf, "panning"); break;
                        case 0xA: strcpy(buf, "stereo control"); break;
                        case 0xC: strcpy(buf, "note cut"); break;
                        case 0xD: strcpy(buf, "note delay"); break;
                        default: 
                            ui_map_effect_num(effect, module_type, effect_num);
                            sprintf(buf, "UNIMPLEMENTED: %s%2x", effect, effect_val); 
                            break;                        
                    }
                    break;
                
                case 20: strcpy(buf, "set tempo"); break;
                
                case 24: strcpy(buf, "panning"); break;
                    
                default: 
                    ui_map_effect_num(effect, module_type, effect_num);
                    sprintf(buf, "UNIMPLEMENTED: %s%2x", effect, effect_val); 
                    break;
                
            }
            break;
    }
        
       
}

void ui_map_effect_num(char * target, const module_type_t type, const uint8_t effect_num)
{
    switch (type) {
        
        case module_type_s3m:
            sprintf(target, "%c", ".ABCDEFGHIJKLMNOPQRSTUVWXYZ"[effect_num]);
            break;

        case module_type_mtm:
        case module_type_mod:
        default:
            sprintf(target, "%X", effect_num);
            break;
    }
}

int ui_lookup_period_index(const module_type_t type, const uint16_t period)
{
    int i;
    
    int num_periods;
    uint16_t * period_table;
    
    switch (type) {
        case module_type_mtm:
        case module_type_mod: 
            num_periods = defs_mod_num_periods; 
            period_table  = (uint16_t *)defs_mod_periods;
            break;
            
        case module_type_s3m:
            num_periods = defs_s3m_num_periods; 
            period_table  = (uint16_t *)defs_s3m_periods;
            break;
            
        default:
            return -1;
            
    }
    
    for (i = 0; i < num_periods; i++) {
        if (period_table[i] == period)
            return i;
    }
    return -1;
}



void ui_generic_order_handler(player_t * player, void * user_ptr) {
    ((ui_dirty_t *) user_ptr)->order = 1;
}

void ui_generic_row_handler(player_t * player, void * user_ptr) {
    ((ui_dirty_t *) user_ptr)->row = 1;
}

void ui_generic_tick_handler(player_t * player, void * user_ptr) {
    ((ui_dirty_t *) user_ptr)->tick = 1;
}

void ui_generic_sample_handler(player_t * player, void * user_ptr) {
    ((ui_dirty_t *) user_ptr)->sample = 1;
}