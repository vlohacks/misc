/* 
 * File:   player_command.h
 * Author: vlo
 *
 * Created on 6. Januar 2014, 00:31
 */

#ifndef PLAYER_COMMAND_H
#define	PLAYER_COMMAND_H

#include "player.h"

typedef enum {
    player_command_action_none,
    player_command_action_next_order,
    player_command_action_prev_order,
    player_command_action_next_song,
    player_command_action_prev_song
} player_command_action_t;

void player_command_next_order(player_t * player);
void player_command_prev_order(player_t * player);

#endif	/* PLAYER_COMMAND_H */

