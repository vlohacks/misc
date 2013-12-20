/* 
 * File:   ui.h
 * Author: vlo
 *
 * Created on 29. Juni 2013, 18:04
 */

#ifndef UI_H
#define	UI_H

#include <stdint.h>
#include "module.h"

void ui_periodindex2note(int period_index, char * dest);
void ui_protracker_effect_to_humanreadable(char * buf, uint8_t effect_num, uint8_t effect_val);
int ui_lookup_period_index(const module_type_t type, const uint16_t period);

#endif	/* UI_H */

