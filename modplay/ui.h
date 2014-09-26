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
#include "player.h"

typedef struct {
    volatile int order : 1;
    volatile int tick : 1;
    volatile int row : 1;
    volatile int sample : 1;
} ui_dirty_t;

void ui_periodindex2note(int period_index, char * dest);
void ui_effect_to_humanreadable(char * buf, const uint8_t effect_num, const uint8_t * effect_values, const module_type_t module_type);
void ui_map_effect_num(char * target, const module_type_t type, const uint8_t effect_num);
int ui_lookup_period_index(const module_type_t type, const uint16_t period);

void ui_generic_order_handler(player_t * player, void * user_ptr);
void ui_generic_row_handler(player_t * player, void * user_ptr);
void ui_generic_tick_handler(player_t * player, void * user_ptr);
void ui_generic_sample_handler(player_t * player, void * user_ptr);

#endif	/* UI_H */

