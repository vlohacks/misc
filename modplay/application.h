/* 
 * File:   application.h
 * Author: vlo
 *
 * Created on 15. Juli 2013, 21:31
 */

#ifndef APPLICATION_H
#define	APPLICATION_H

#include "player.h"
#include "output.h"
#include "ui.h"

typedef enum {
    ui_flavour_quiet,
    ui_flavour_terminal,        
    ui_flavour_curses
} ui_flavour_t;

typedef struct {
    player_t * player;
    output_opts_t * output_opts;
    char * output_config;
    char ** playlist;
    int playlist_count;
    int loop_playlist;
    int running;
    ui_flavour_t ui_flavour;
    ui_dirty_t ui_dirty;
} modplay_application_t;

#endif	/* APPLICATION_H */

