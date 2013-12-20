#include "ui.h"
#include "module.h"
#include <string.h>
#include <stdio.h>

#include "defs_mod.h"
#include "defs_s3m.h"

void ui_periodindex2note(int period_index, char * dest)
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

    //i = protracker_lookup_period_index(period);
    if (period_index >= 8)
        sprintf(dest, "%s%1u", notes[i % 12], i / 12);
    
}

void ui_protracker_effect_to_humanreadable(char * buf, uint8_t effect_num, uint8_t effect_val)
{
    *buf = 0;
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
                case 0x6: strcpy(buf, "pattern loop"); break;
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
    
}

int ui_lookup_period_index(const module_type_t type, const uint16_t period)
{
    int i;
    
    int num_periods;
    uint16_t * period_table;
    
    switch (type) {
        case module_type_mod: 
            num_periods = defs_mod_num_periods; 
            period_table  = defs_mod_periods;
            break;
            
        case module_type_s3m:
            num_periods = defs_s3m_num_periods; 
            period_table  = defs_s3m_periods;
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

