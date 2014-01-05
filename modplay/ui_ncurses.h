/* 
 * File:   ui_ncurses.h
 * Author: vlo
 *
 * Created on 16. Juli 2013, 18:34
 */

#ifndef UI_NCURSES_H
#define	UI_NCURSES_H

#include <ncurses.h>
#include "module.h"
#include "player.h"

typedef struct  {
    int init;
    int ncurses_inited;
    WINDOW * song_view;
    WINDOW * channel_view;
    WINDOW * pattern_view;
    int current_w;
    int current_h;
    int num_channels;
} ui_ncurses_layout_t;

void ui_ncurses_init();
void ui_ncurses_cleanup();
void ui_ncurses_term_resized(int i);
void ui_ncurses_layout_init();
void ui_ncurses_new_song_handler(module_t * mod);
void ui_ncurses_order_handler(player_t * player, int current_order, int current_pattern);
void ui_ncurses_tick_handler(player_t * player, int current_order, int current_pattern, int current_row, int current_tick, player_channel_t * channels);
void ui_ncurses_row_handler(player_t * player, int current_order, int current_pattern, int current_row);
void ui_ncurses_channel_sample_handler(float l, float r, float peak_l, float peak_r, int channel);

#endif	/* UI_NCURSES_H */

