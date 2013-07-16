/* 
 * File:   ui.h
 * Author: vlo
 *
 * Created on 29. Juni 2013, 18:04
 */

#ifndef UI_H
#define	UI_H

#include <stdint.h>

void ui_period2note(uint16_t period, char * dest);
void ui_protracker_effect_to_humanreadable(char * buf, uint8_t effect_num, uint8_t effect_val);


#endif	/* UI_H */

