#ifndef EFFECTS_STM_H
#define	EFFECTS_STM_H

#include "player.h"
#include "module.h"

static const int effects_stm_map_size = 11;

effect_callback_t * effects_stm_init();
void effects_stm_newrowaction(player_t *, module_pattern_data_t *, int);
uint16_t effects_stm_get_tuned_period(player_t * player, uint16_t base_period, int channel);
void effects_stm_A_settempo(player_t * player, int channel);
void effects_stm_B_positionjump(player_t * player, int channel);
void effects_stm_C_patternbreak(player_t * player, int channel);
void effects_stm_D_volumeslide(player_t * player, int channel);
void effects_stm_E_slidedown(player_t * player, int channel);
void effects_stm_F_slideup(player_t * player, int channel);
void effects_stm_G_slidetonote(player_t * player, int channel);
void effects_stm_H_vibrato(player_t * player, int channel);
void effects_stm_I_tremor(player_t * player, int channel);
void effects_stm_J_arpeggio(player_t * player, int channel_num);

void effects_stm_unimplemented(player_t * player, int channel);
#endif	/* EFFECTS_STM_H */

