#include "player_command.h"


void player_command_action_dispatch(player_t * player, player_command_action_t action)
{
    switch(action) {
        case player_command_action_next_order: player_command_next_order(player); break;
        case player_command_action_prev_order: player_command_prev_order(player); break;
        default: break;
    }
}

/* skip player to begin of next order */
void player_command_next_order(player_t * player)
{
    if (player->current_order < (player->module->num_orders - 1)) 
        player->next_order = player->current_order + 1;
    
    player->next_row = 0;
    player->do_break = 1;
    
}

/* skip player to begin of previous order */
void player_command_prev_order(player_t * player)
{
    if (player->current_order > 0) 
        player->next_order = player->current_order - 1;
    else if (player->current_order == 0)
        player->next_order = 0;
    
    player->next_row = 0;
    player->do_break = 1;
    
}

