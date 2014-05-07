/* 
 * File:   ui_ncurses.h
 * Author: vlo
 *
 * Created on 16. Juli 2013, 18:34
 */

#ifndef UI_NCURSES_H
#define	UI_NCURSES_H

#include <ncurses.h>
//#include "curses.h"
#include "module.h"
#include "player.h"
#include "player_command.h"
#include "mixing.h"

#define UI_NCURSES_COLORPAIR_WINDOW 1

typedef struct  {
    int init;
    int ncurses_inited;
    int use_colors;
    WINDOW * song_view;
    WINDOW * channel_view;
    WINDOW * pattern_view;
    WINDOW * aux;
    int current_w;
    int current_h;
    int num_channels;
} ui_ncurses_layout_t;

typedef struct {
    char * keyseq;
    char * keyname;
    char * description;
    player_command_action_t action;
    void (*hook)();
} ui_ncurses_keybinding_t;

void ui_ncurses_init();
void ui_ncurses_cleanup();
void ui_ncurses_term_resized(int i);
void ui_ncurses_layout_init();
void ui_ncurses_new_song_handler(module_t * mod);
void ui_ncurses_order_handler(player_t * player, int current_order, int current_pattern);
void ui_ncurses_tick_handler(player_t * player, int current_order, int current_pattern, int current_row, int current_tick, player_channel_t * channels);
void ui_ncurses_row_handler(player_t * player, int current_order, int current_pattern, int current_row);
void ui_ncurses_channel_sample_handler(sample_t l, sample_t r, sample_t peak_l, sample_t peak_r, int channel);
player_command_action_t ui_ncurses_handle_input();
void ui_ncurses_show_log();
void ui_ncurses_show_help();

#endif	/* UI_NCURSES_H */

