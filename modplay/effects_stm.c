#include "effects_stm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

effect_callback_t * effects_stm_init()
{
    int i;
    effect_callback_t * effect_map = malloc(sizeof(effect_callback_t) * effects_stm_map_size);
    
    // initialize with default: unimplemented
    for (i = 0; i < effects_stm_map_size; i++)
        effect_map[i] = effects_stm_unimplemented;
    
    effect_map[0] = 0;                                  // 0 means no effect
    effect_map[1] = effects_stm_A_settempo;
    effect_map[2] = effects_stm_B_positionjump;
    effect_map[3] = effects_stm_C_patternbreak;
    effect_map[4] = effects_stm_D_volumeslide;
    effect_map[5] = effects_stm_E_slidedown;
    effect_map[6] = effects_stm_F_slideup;
    effect_map[7] = effects_stm_G_slidetonote;
    effect_map[8] = effects_stm_H_vibrato;
    effect_map[9] = effects_stm_I_tremor;
    effect_map[10] = effects_stm_J_arpeggio;
    
    return effect_map;
}

void effects_stm_newrowaction(player_t * player, module_pattern_data_t * data, int channel_num)
{
    
    // special behaviour for sample / note delay
    if ((data->effect_num == 19) && ((data->effect_value >> 4) == 0xd)) {
        if (data->period_index >= 0) {
                //player->channels[channel_num].dest_period = player->period_table[data->period_index];
                player->channels[channel_num].dest_period = effects_stm_get_tuned_period(player, player->period_table[data->period_index], channel_num);
                player->channels[channel_num].dest_sample_num = data->sample_num;
                player->channels[channel_num].dest_volume = data->volume;
        } else {
            player->channels[channel_num].dest_period = 0;
        }
        return;
    }
    
    // set sample
    if (data->sample_num > 0) {
        player->channels[channel_num].sample_num = data->sample_num;
        player->channels[channel_num].volume = player->module->samples[player->channels[channel_num].sample_num - 1].header.volume;
    }
    
    // set volume
    if (data->volume >= 0) {
        player->channels[channel_num].volume = data->volume;
    }

    // set period (note)
    if (data->period_index >= 0) {
        player->channels[channel_num].period_index = data->period_index;
        if (data->period_index == 254) { // note off
            player->channels[channel_num].sample_num = 0;
            return;
        } else {
            // special hack for note portamento... TODO remove here
            if (data->effect_num == 0x7) {
                //player->channels[channel_num].dest_period = player->period_table[data->period_index];
                player->channels[channel_num].dest_period = effects_stm_get_tuned_period(player, player->period_table[data->period_index], channel_num);
            } else {
                player->channels[channel_num].dest_period = 0;
                if (!(player->pattern_delay_active)) {
                    //player->channels[channel_num].period =  player->period_table[data->period_index];
                    player->channels[channel_num].period = effects_stm_get_tuned_period(player, player->period_table[data->period_index], channel_num);
                    player->channels[channel_num].sample_pos = 0;
                    //player_channel_set_frequency(player, player->channels[channel_num].period, channel_num);
                }
            }
        }
    } 
    
    //player->channels[channel_num].tremolo_state = 0;
    player->channels[channel_num].volume_master = 64;
    
    // do not set frequency for tone portamento effects
    if ((data->effect_num != 0x7) && (data->effect_num != 0x5) && (data->effect_num != 0x6) && (data->effect_num != 0x8)) {
        if (data->period_index >= 0)
            player_channel_set_frequency(player, player->channels[channel_num].period, channel_num);
    }
}

uint16_t effects_stm_get_tuned_period(player_t * player, uint16_t base_period, int channel)
{
    if (player->channels[channel].sample_num == 0)
        return 1;
    
    module_sample_t * sample = &(player->module->samples[player->channels[channel].sample_num - 1]);
    return (base_period * 8363) / sample->header.c2spd;
}






void effects_stm_A_settempo(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        player->speed = player->channels[channel].effect_value >> 4;
    }    
}

void effects_stm_B_positionjump(player_t * player, int channel)
{
    
    // prevent egoist-mods from looping back if no single-mod-loop is enabled
    if (player->channels[channel].effect_value < player->current_order) {
        if (!player->loop_module)
            return;
    }
        
    if (player->current_tick == 0) {
        player->next_order = player->channels[channel].effect_value;
        player->next_row = 0;
        player->do_break = 1;
    }
}

void effects_stm_C_patternbreak(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        player->next_row = ((player->channels[channel].effect_value >> 4) * 10 + (player->channels[channel].effect_value & 0x0f));
        player->do_break = 1;
    }
}

void effects_stm_D_volumeslide(player_t * player, int channel)
{
    int tmp;
    
    if (player->current_tick == 0) {
        /* remember last volume slide command parameter */
        if (player->channels[channel].effect_value)
            player->channels[channel].effect_last_value[4] = player->channels[channel].effect_value;
        
        return;
    }

    if ((player->channels[channel].effect_last_value[4] & 0x0f) == 0x00) {
        tmp = player->channels[channel].volume + (player->channels[channel].effect_last_value[4] >> 4);
        if (tmp > 64)
            tmp = 64;
        player->channels[channel].volume = tmp;
    } else if ((player->channels[channel].effect_last_value[4] & 0xf0) == 0x00) {
        tmp = (int)player->channels[channel].volume - (int)(player->channels[channel].effect_last_value[4] & 0x0f);
        if (tmp < 0)
            tmp = 0;
        player->channels[channel].volume = tmp;
    }    
}

void effects_stm_E_slidedown(player_t * player, int channel)
{
    int tmp;
    
    if (player->current_tick == 0) {
        /* remember last slide down command parameter */
        if (player->channels[channel].effect_value) {
            player->channels[channel].effect_last_value[5] = player->channels[channel].effect_value;
            player->channels[channel].effect_last_value[6] = player->channels[channel].effect_last_value[5];
        }
        
        return;
    }
    
    if ((player->channels[channel].effect_last_value[5]) < 0xe0) {
        tmp = (int)player->channels[channel].period + ((player->channels[channel].effect_last_value[5]) << 2);
        player->channels[channel].period = tmp;
        player_channel_set_frequency(player, player->channels[channel].period, channel);
    }
    
}

void effects_stm_F_slideup(player_t * player, int channel)
{
    int tmp;
    
    if (player->current_tick == 0) {
        /* remember last slide down command parameter */
        if (player->channels[channel].effect_value) {
            player->channels[channel].effect_last_value[6] = player->channels[channel].effect_value;
            player->channels[channel].effect_last_value[5] = player->channels[channel].effect_last_value[6];
        }
        
        return;
    }

    if ((player->channels[channel].effect_last_value[6]) < 0xe0) {
        tmp = (int)player->channels[channel].period - ((player->channels[channel].effect_last_value[6]) << 2);
        player->channels[channel].period = tmp;
        player_channel_set_frequency(player, player->channels[channel].period, channel);
    }
    
}


void effects_stm_G_slidetonote(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        if (player->channels[channel].effect_value) 
            player->channels[channel].effect_last_value[7] = player->channels[channel].effect_value;
        return;        
    }
    
    if (player->channels[channel].dest_period == 0)
        return;
    
    if (player->channels[channel].period > player->channels[channel].dest_period) {
        int tmp = (int)player->channels[channel].period - ((int)player->channels[channel].effect_last_value[7] << 2);
        if (tmp < player->channels[channel].dest_period)
            tmp = player->channels[channel].dest_period;
        player->channels[channel].period = (uint16_t)tmp;
    } else if (player->channels[channel].period < player->channels[channel].dest_period) {
        player->channels[channel].period += (player->channels[channel].effect_last_value[7] << 2);
        if (player->channels[channel].period > player->channels[channel].dest_period)
            player->channels[channel].period = player->channels[channel].dest_period;
    }
    player_channel_set_frequency(player, player->channels[channel].period, channel);
    
}


void effects_stm_H_vibrato(player_t * player, int channel) 
{
    uint8_t temp;
    uint16_t delta;
    
    if (player->current_tick == 0) {
        if (player->channels[channel].vibrato_waveform < 4)
            player->channels[channel].vibrato_state = 0;
        
        if ((player->channels[channel].effect_value >> 4) != 0x00) 
            player->channels[channel].effect_last_value[player->channels[channel].effect_num] = (player->channels[channel].effect_value >> 4);

        if ((player->channels[channel].effect_value & 0xf) != 0x00) 
            player->channels[channel].effect_last_value_y[player->channels[channel].effect_num] = (player->channels[channel].effect_value & 0xf);
        
        return;    
    }
    
    temp = player->channels[channel].vibrato_state & 0x1f;
    
    delta = defs_mod_sine_table[temp]; 
    
    delta *= (player->channels[channel].effect_last_value_y[player->channels[channel].effect_num]);
    delta /= 32; // (128 / 4 = 32) - due to 4 times bigger periods in s3m compared to protracker
    
    if (player->channels[channel].vibrato_state >= 0)
        player_channel_set_frequency(player, player->channels[channel].period + delta, channel);
    else
        player_channel_set_frequency(player, player->channels[channel].period - delta, channel);
    
    player->channels[channel].vibrato_state += player->channels[channel].effect_last_value[player->channels[channel].effect_num];
    if (player->channels[channel].vibrato_state > 31)
        player->channels[channel].vibrato_state -= 64;
}


void effects_stm_I_tremor(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        if (player->channels[channel].effect_value) {
            /*
            if (player->channels[channel].effect_last_value[9] != player->channels[channel].current_effect_value)
                player->channels[channel].tremor_state = 0;
             */
            player->channels[channel].effect_last_value[9] = player->channels[channel].effect_value;
        }
    }
    
    uint8_t tremor_on = (player->channels[channel].effect_last_value[9] >> 4) + 1;
    uint8_t tremor_off = (player->channels[channel].effect_last_value[9] & 0x0f) + 1;
    
    if (player->channels[channel].tremor_state < tremor_on)
        player->channels[channel].volume_master = 64;
    else
        player->channels[channel].volume_master = 0;
                    
    player->channels[channel].tremor_state++;
    if (player->channels[channel].tremor_state > (tremor_on + tremor_off))
        player->channels[channel].tremor_state = 0;
    
            
}


void effects_stm_J_arpeggio(player_t * player, int channel_num)
{
    player_channel_t * channel = &(player->channels[channel_num]);
    
    // 000 means no effect - therefore also no arpeggio
    if (channel->effect_value == 0)     
        return;
    
    uint8_t temp = player->current_tick % 3;
    //protracker_lookup_period_index(channel->period);
    int temp2 = channel->period;
    
    // arpeggio without note makes no sense (bladswed.mod))
    if (!channel->period)
        return;
    
    switch (temp) {
    case 0:
        //temp2 = protracker_periods[temp2];
        break;
    case 1:
        //temp2 = protracker_periods[temp2 + (channel->current_effect_value >> 4)];
        temp2 *= pow(2.0f, (float)(channel->effect_value >> 4) / -12.0f);
        break;
    case 2:
        //temp2 = protracker_periods[temp2 + (channel->current_effect_value & 0x0f)];
        temp2 *= pow(2.0f, (float)(channel->effect_value & 0xf) / -12.0f);
        break;
    }
    player_channel_set_frequency(player, (uint16_t)temp2, channel_num);
    
}



void effects_stm_unimplemented(player_t * player, int channel)
{
/*
    char effect[2];
    // only alert once per row
    if (player->current_tick == 0) {
        ui_map_effect_num(effect, player->module->module_type, player->channels[channel].current_effect_num);
        fprintf(stderr, "\nUnimplemented: %s%02x\n", effect, player->channels[channel].current_effect_value);
    }
 */
   
}