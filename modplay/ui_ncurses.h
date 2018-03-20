/* 
 * File:   ui_ncurses.h
 * Author: vlo
 *
 * Created on 16. Juli 2013, 18:34
 */

#ifndef UI_NCURSES_H
#define	UI_NCURSES_H

#include "platform.h"

#ifdef PLATFORM_POSIX
#include <curses.h>
#else
#include <curses.h>
#endif
#include <panel.h>
//#include "curses.h"
#include "ui.h"
#include "module.h"
#include "player.h"
#include "player_command.h"
#include "mixing.h"

#define UI_NCURSES_COLORPAIR_WINDOW 1

typedef struct  {
    int init;
    int ncurses_inited;
    int use_colors;
    struct {
        WINDOW * w;    
        PANEL * p;
    } song_panel;
    struct {
        WINDOW * w;
        PANEL * p;
    } channel_panel;
    struct {
        WINDOW * w;
        PANEL * p;
    } pattern_panel;
    struct {
        WINDOW * w;
        PANEL * p;
        int list_offset;
    } samples_panel;
    struct {
        WINDOW * w;
        PANEL * p;
    } aux_panel;
    int current_w;
    int current_h;
    int num_channels;
    char filename[128];
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
void ui_ncurses_windows_init() ;
void ui_ncurses_system_init();
void ui_ncurses_new_song_handler(module_t * mod);
void ui_ncurses_order_handler(player_t * player);
void ui_ncurses_tick_handler(player_t * player);
void ui_ncurses_row_handler(player_t * player);
void ui_ncurses_channel_sample_handler(sample_t l, sample_t r, sample_t peak_l, sample_t peak_r, int channel);
player_command_action_t ui_ncurses_handle_input();
void ui_ncurses_show_log();
void ui_ncurses_show_help();
void ui_ncurses_refresh(player_t * player, ui_dirty_t * ui_dirty);
void ui_ncurses_set_filename(char * filename);
void ui_ncurses_scrolldown();
void ui_ncurses_scrollup();

#endif	/* UI_NCURSES_H */

