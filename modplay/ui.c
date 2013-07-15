#include "ui.h"
#include "protracker.h"
#include <string.h>
#include <stdio.h>

void ui_period2note(uint16_t period, char * dest)
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

    i = protracker_lookup_period_index(period);
    if (i >= 8)
        sprintf(dest, "%s%1u", notes[i % 12], i / 12);
    
}
