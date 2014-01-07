/* 
 * File:   effects_mod.h
 * Author: vlo
 *
 * Created on 30. Juni 2013, 00:11
 */

#ifndef EFFECTS_PROTRACKER_H
#define	EFFECTS_PROTRACKER_H

#include "player.h"
#include "module.h"

static const int effects_mod_map_size = 16;

effect_callback_t * effects_mod_init();

void effects_mod_newrowaction(player_t *, module_pattern_data_t *, int);

void effects_mod_0_arpeggio(player_t *, int);
void effects_mod_1_slideup(player_t *, int);
void effects_mod_2_slidedown(player_t *, int);
void effects_mod_3_slidetonote(player_t *, int);
void effects_mod_4_vibrato(player_t *, int);
void effects_mod_5_slidetonote_volumeslide(player_t *, int);
void effects_mod_6_vibrato_volumeslide(player_t *, int);
void effects_mod_7_tremolo(player_t *, int);
void effects_mod_8_panning(player_t *, int);                                    // non-protracker standard
void effects_mod_9_sampleoffset(player_t *, int);
void effects_mod_a_volumeslide(player_t *, int);
void effects_mod_b_positionjump(player_t *, int);
void effects_mod_c_setvolume(player_t *, int);
void effects_mod_d_patternbreak(player_t *, int);
void effects_mod_e_special(player_t *, int);
void effects_mod_e1_fineslideup(player_t *, int);
void effects_mod_e2_fineslidedown(player_t *, int);
void effects_mod_e4_setvibratowaveform(player_t * player, int channel);
void effects_mod_e6_patternloop(player_t *, int);
void effects_mod_e7_settremolowaveform(player_t * player, int channel);
void effects_mod_e8_panning(player_t * player, int channel);                    // non-protracker standard
void effects_mod_e9_retriggersample(player_t *, int);
void effects_mod_ea_finevolumeup(player_t *, int);
void effects_mod_eb_finevolumedown(player_t *, int);
void effects_mod_ec_notecut(player_t *, int);
void effects_mod_ed_delaysample(player_t * player, int channel);
void effects_mod_ee_patterndelay(player_t *, int);
void effects_mod_f_setspeed(player_t *, int);

void effects_mod_unimplemented(player_t *, int);

#endif	/* EFFECTS_PROTRACKER_H */

