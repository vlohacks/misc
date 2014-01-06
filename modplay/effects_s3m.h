/* 
 * File:   effects_s3m.h
 * Author: vlo
 *
 * Created on 24. Dezember 2013, 12:47
 */

#ifndef EFFECTS_S3M_H
#define	EFFECTS_S3M_H


#include "player.h"
#include "module.h"

static const int effects_s3m_map_size = 26;

effect_callback_t * effects_s3m_init();
void effects_s3m_newrowaction(player_t *, module_pattern_data_t *, int);
uint16_t effects_s3m_get_tuned_period(player_t * player, uint16_t base_period, int channel);
void effects_s3m_A_setspeed(player_t * player, int channel);
void effects_s3m_B_positionjump(player_t * player, int channel);
void effects_s3m_C_patternbreak(player_t * player, int channel);
void effects_s3m_D_volumeslide(player_t * player, int channel);
void effects_s3m_E_slidedown(player_t * player, int channel);
void effects_s3m_F_slideup(player_t * player, int channel);
void effects_s3m_G_slidetonote(player_t * player, int channel);
void effects_s3m_H_vibrato(player_t * player, int channel);
void effects_s3m_I_tremor(player_t * player, int channel);
void effects_s3m_J_arpeggio(player_t * player, int channel_num);
void effects_s3m_K_vibrato_volumeslide(player_t * player, int channel);
void effects_s3m_L_slidetonote_volumeslide(player_t * player, int channel);
void effects_s3m_O_sampleoffset(player_t * player, int channel);
void effects_s3m_Q_retrigger_volumeslide(player_t * player, int channel);
void effects_s3m_R_tremolo(player_t * player, int channel) ;
void effects_s3m_S_special(player_t * player, int channel);
void effects_s3m_S8_panning(player_t * player, int channel);
void effects_s3m_SD_delaysample(player_t * player, int channel);
void effects_s3m_T_setbpm(player_t * player, int channel);
void effects_s3m_X_panning(player_t * player, int channel) ;

void effects_s3m_unimplemented(player_t * player, int channel);
#endif	/* EFFECTS_S3M_H */

