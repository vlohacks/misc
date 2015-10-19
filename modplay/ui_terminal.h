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
#include "ui.h"

/* Prototypes
 */
void ui_terminal_init();
void ui_terminal_print_moduleinfo(module_t * module);
void ui_terminal_print_order_info(player_t * player);
void ui_terminal_print_row_info(player_t * player);
void ui_terminal_refresh(player_t * player, ui_dirty_t * ui_dirty);

#endif	/* UI_TERMINAL_H */

