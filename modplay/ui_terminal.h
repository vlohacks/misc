/* 
 * File:   ui_terminal.h
 * Author: vlo
 *
 * Created on 29. Juni 2013, 20:40
 */

#ifndef UI_TERMINAL_H
#define	UI_TERMINAL_H

#include "module.h"
#include "player.h"

/* Prototypes
 */
void ui_terminal_init();
void ui_terminal_print_moduleinfo(module_t * module);
void ui_terminal_print_order_info(player_t * player, int current_order, int current_pattern);
void ui_terminal_print_row_info(player_t * player, int current_order, int current_pattern, int current_row);

#endif	/* UI_TERMINAL_H */

