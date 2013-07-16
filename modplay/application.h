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

typedef struct {
    player_t * player;
    output_opts_t * output_opts;
    char ** playlist;
    int playlist_count;
    int loop_playlist;
    int running;
} modplay_application_t;

#endif	/* APPLICATION_H */

