
#include "effects_s3m.h"
#include <stdio.h>
#include <stdlib.h>

effect_callback_t * effects_s3m_init()
{
    int i;
    effect_callback_t * effect_map = malloc(sizeof(effect_callback_t) * effects_s3m_map_size);
    
    // initialize with default: unimplemented
    for (i = 0; i < effects_s3m_map_size; i++)
        effect_map[i] = effects_s3m_unimplemented;
    
    effect_map[0] = 0;                                  // 0 means no effect
    effect_map[1] = effects_s3m_A_setspeed;
    effect_map[3] = effects_s3m_C_patternbreak;
    effect_map[4] = effects_s3m_D_volumeslide;
    effect_map[5] = effects_s3m_E_slidedown;
    effect_map[6] = effects_s3m_F_slideup;
    effect_map[7] = effects_s3m_G_slidetonote;
    effect_map[8] = effects_s3m_H_vibrato;
    effect_map[11] = effects_s3m_K_vibrato_volumeslide;
    
    effect_map[15] = effects_s3m_O_sampleoffset;
    /*
    effect_map[16] = effects_s3m_P_sampleoffset;
    effect_map[17] = effects_s3m_Q_sampleoffset;
    effect_map[18] = effects_s3m_R_sampleoffset;
    */
    effect_map[19] = effects_s3m_S_special;
    effect_map[20] = effects_s3m_T_setbpm;
    
    return effect_map;
}

void effects_s3m_newrowaction(player_t * player, module_pattern_data_t * data, int channel_num)
{
    
    // special behaviour for sample / note delay
    if ((data->effect_num == 0xe) && ((data->effect_value >> 4) == 0xd)) {
        if (data->period_index >= 0) {
                //player->channels[channel_num].dest_period = player->period_table[data->period_index];
                player->channels[channel_num].dest_period = effects_s3m_get_tuned_period(player, player->period_table[data->period_index], channel_num);
                player->channels[channel_num].dest_sample_num = data->sample_num;
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
                player->channels[channel_num].dest_period = effects_s3m_get_tuned_period(player, player->period_table[data->period_index], channel_num);
            } else {
                player->channels[channel_num].dest_period = 0;
                if (!(player->pattern_delay_active)) {
                    //player->channels[channel_num].period =  player->period_table[data->period_index];
                    player->channels[channel_num].period = effects_s3m_get_tuned_period(player, player->period_table[data->period_index], channel_num);
                    player->channels[channel_num].sample_pos = 0;
                    //player_channel_set_frequency(player, player->channels[channel_num].period, channel_num);
                }
            }
        }
    } 
    
    //player->channels[channel_num].vibrato_state = 0;
    //player->channels[channel_num].tremolo_state = 0;
    player->channels[channel_num].volume_master = 64;
    
    // do not set frequency for tone portamento effects
    if ((data->effect_num) != 0x7 && (data->effect_num != 0x5)) {
        if (data->period_index >= 0)
            player_channel_set_frequency(player, player->channels[channel_num].period, channel_num);
    }
}

uint16_t effects_s3m_get_tuned_period(player_t * player, uint16_t base_period, int channel)
{
    if (player->channels[channel].sample_num == 0)
        return 1;
    
    module_sample_t * sample = &(player->module->samples[player->channels[channel].sample_num - 1]);
    return (base_period * 8363) / sample->header.c2spd;
}


void effects_s3m_A_setspeed(player_t * player, int channel)
{
    if (player->channels[channel].current_effect_value == 0)
        return;
            
    if (player->current_tick == 0) 
        player->speed = player->channels[channel].current_effect_value;
    
}

void effects_s3m_C_patternbreak(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        player->next_row = ((player->channels[channel].current_effect_value >> 4) * 10 + (player->channels[channel].current_effect_value & 0x0f));
        player->do_break = 1;
    }
}

void effects_s3m_D_volumeslide(player_t * player, int channel)
{
    int tmp;
    
    if (player->current_tick == 0) {
        /* remember last volume slide command parameter */
        if (player->channels[channel].current_effect_value)
            player->channels[channel].effect_last_value[4] = player->channels[channel].current_effect_value;
        
        /* process fine volume slides only on first tick */
        if ((player->channels[channel].effect_last_value[4] & 0x0f) == 0x0f) {
            tmp = player->channels[channel].volume + (player->channels[channel].effect_last_value[4] >> 4);
            if (tmp > 64)
                tmp = 64;
            player->channels[channel].volume = tmp;
        } else if ((player->channels[channel].effect_last_value[4] & 0xf0) == 0xf0) {
            tmp = (int)player->channels[channel].volume - (int)(player->channels[channel].effect_last_value[4] & 0x0f);
            if (tmp < 0)
                tmp = 0;
            player->channels[channel].volume = tmp;
        }
        
        /* buggy st3.0 volumeslides, also processed on first tick */
        if (!player->module->module_info.flags_s3m.st30volumeslides)
            return;
    }
    
    /* process regular volume slides */
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

void effects_s3m_E_slidedown(player_t * player, int channel)
{
    int tmp;
    
    if (player->current_tick == 0) {
        /* remember last slide down command parameter */
        if (player->channels[channel].current_effect_value)
            player->channels[channel].effect_last_value[5] = player->channels[channel].current_effect_value;
        
        if ((player->channels[channel].effect_last_value[5] & 0xf0) == 0xf0) {
            /* fine portamento */
            tmp = (int)player->channels[channel].period + ((player->channels[channel].effect_last_value[5] & 0x0f) << 2);
            if (tmp > player->period_top)
                tmp = player->period_top;
            player->channels[channel].period = tmp;
            player_channel_set_frequency(player, player->channels[channel].period, channel);
        } else if ((player->channels[channel].effect_last_value[5] & 0xf0) == 0xe0) {
            /* extra fine portamento */
            tmp = (int)player->channels[channel].period + (player->channels[channel].effect_last_value[5] & 0x0f);
            if (tmp > player->period_top)
                tmp = player->period_top;
            player->channels[channel].period = tmp;
            player_channel_set_frequency(player, player->channels[channel].period, channel);
            printf("extrafine %i\n", player->channels[channel].period);
        }
        
        return;
    }
    
    if ((player->channels[channel].effect_last_value[5]) < 0xe0) {
        /* regular portamento */
        tmp = (int)player->channels[channel].period + ((player->channels[channel].effect_last_value[5]) << 2);
        if (tmp > player->period_top)
            tmp = player->period_top;
        player->channels[channel].period = tmp;
        player_channel_set_frequency(player, player->channels[channel].period, channel);
    }
    
}

void effects_s3m_F_slideup(player_t * player, int channel)
{
    int tmp;
    
    /* notice:
     * effect_last_value[5] is NO copy paste bug! Slide Up / Down share one
     * saved parameter!!!
     */
    
    if (player->current_tick == 0) {
        /* remember last slide down command parameter */
        if (player->channels[channel].current_effect_value)
            player->channels[channel].effect_last_value[5] = player->channels[channel].current_effect_value;
        
        if ((player->channels[channel].effect_last_value[5] & 0xf0) == 0xf0) {
            /* fine portamento */
            tmp = (int)player->channels[channel].period - ((player->channels[channel].effect_last_value[5] & 0x0f) << 2);
            if (tmp < player->period_bottom)
                tmp = player->period_bottom;
            player->channels[channel].period = tmp;
            player_channel_set_frequency(player, player->channels[channel].period, channel);
        } else if ((player->channels[channel].effect_last_value[5] & 0xf0) == 0xe0) {
            /* extra fine portamento */
            tmp = (int)player->channels[channel].period - ((player->channels[channel].effect_last_value[5] & 0x0f));
            if (tmp < player->period_bottom)
                tmp = player->period_bottom;
            player->channels[channel].period = tmp;
            player_channel_set_frequency(player, player->channels[channel].period, channel);            
        }
        
        return;
    }
    
    if ((player->channels[channel].effect_last_value[5]) < 0xe0) {
        /* regular portamento */
        tmp = (int)player->channels[channel].period - ((player->channels[channel].effect_last_value[5]) << 2);
        if (tmp < player->period_bottom)
            tmp = player->period_bottom;
        player->channels[channel].period = tmp;
        player_channel_set_frequency(player, player->channels[channel].period, channel);
    }
    
}


void effects_s3m_G_slidetonote(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        if (player->channels[channel].current_effect_value) 
            player->channels[channel].effect_last_value[7] = player->channels[channel].current_effect_value;
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


void effects_s3m_H_vibrato(player_t * player, int channel) 
{
    uint8_t temp;
    uint16_t delta;
    
    if (player->current_tick == 0) {
        if ((player->channels[channel].current_effect_value >> 4) != 0x00) 
            player->channels[channel].effect_last_value[player->channels[channel].current_effect_num] = (player->channels[channel].current_effect_value >> 4);

        if ((player->channels[channel].current_effect_value & 0xf) != 0x00) 
            player->channels[channel].effect_last_value_y[player->channels[channel].current_effect_num] = (player->channels[channel].current_effect_value & 0xf);
        
        return;    
    }
    

    temp = player->channels[channel].vibrato_state & 0x1f;
    delta = defs_mod_sine_table[temp];
    
    delta *= (player->channels[channel].effect_last_value_y[player->channels[channel].current_effect_num]);
    delta /= 32; // (128 / 4 = 32) - due to 4 times bigger periods in s3m compared to protracker
    
    if (player->channels[channel].vibrato_state >= 0)
        player_channel_set_frequency(player, player->channels[channel].period + delta, channel);
    else
        player_channel_set_frequency(player, player->channels[channel].period - delta, channel);
    
    player->channels[channel].vibrato_state += player->channels[channel].effect_last_value[player->channels[channel].current_effect_num];
    if (player->channels[channel].vibrato_state > 31)
        player->channels[channel].vibrato_state -= 64;
}


void effects_s3m_K_vibrato_volumeslide(player_t * player, int channel) 
{
    int tmp;
    uint8_t temp;
    uint16_t delta;
    
    if (player->current_tick == 0) {
        if (player->channels[channel].current_effect_value)
            player->channels[channel].effect_last_value[11] = player->channels[channel].current_effect_value;

        return;    
    }
    

    /* maintain vibrato */
    temp = player->channels[channel].vibrato_state & 0x1f;
    delta = defs_mod_sine_table[temp];
    
    delta *= (player->channels[channel].effect_last_value_y[8]);
    delta /= 32; // (128 / 4 = 32) - due to 4 times bigger periods in s3m compared to protracker
    
    if (player->channels[channel].vibrato_state >= 0)
        player_channel_set_frequency(player, player->channels[channel].period + delta, channel);
    else
        player_channel_set_frequency(player, player->channels[channel].period - delta, channel);
    
    player->channels[channel].vibrato_state += player->channels[channel].effect_last_value[8];
    if (player->channels[channel].vibrato_state > 31)
        player->channels[channel].vibrato_state -= 64;
    
    /* do the volume slide */
    if ((player->channels[channel].effect_last_value[11] & 0x0f) == 0x00) {
        tmp = player->channels[channel].volume + (player->channels[channel].effect_last_value[11] >> 4);
        if (tmp > 64)
            tmp = 64;
        player->channels[channel].volume = tmp;
    } else if ((player->channels[channel].effect_last_value[11] & 0xf0) == 0x00) {
        tmp = (int)player->channels[channel].volume - (int)(player->channels[channel].effect_last_value[11] & 0x0f);
        if (tmp < 0)
            tmp = 0;
        player->channels[channel].volume = tmp;
    }      
    
}


void effects_s3m_O_sampleoffset(player_t * player, int channel)
{
    if (player->current_tick == 0) {
        if (player->channels[channel].current_effect_value)
            player->channels[channel].effect_last_value[15] = player->channels[channel].current_effect_value;
        
        player->channels[channel].sample_pos = player->channels[channel].effect_last_value[15] << 8;
        if (player->channels[channel].sample_pos > player->module->samples[player->channels[channel].sample_num - 1].header.length - 1) 
            player->channels[channel].sample_pos = player->module->samples[player->channels[channel].sample_num - 1].header.length - 1;
    }
}


void effects_s3m_S_special(player_t * player, int channel)
{
    switch (player->channels[channel].current_effect_value >> 4) {
        case 0x8: effects_s3m_S8_stereocontrol(player, channel); break;
        default: effects_s3m_unimplemented(player, channel);
    }
}


void effects_s3m_S8_stereocontrol(player_t * player, int channel) 
{
    if (player->current_tick == 0) {
        uint8_t i = player->channels[channel].current_effect_value & 0x0f;
        player->channels[channel].panning = (i << 4) | ((i << 1) + (i>6?1:0));
    }
}

void effects_s3m_T_setbpm(player_t * player, int channel)
{
    if (player->current_tick == 0) {
         if (player->channels[channel].current_effect_value >= 0x20) {
            player->bpm = player->channels[channel].current_effect_value;
            player->tick_duration = player_calc_tick_duration(player->bpm, player->sample_rate);
         }
    }
}


void effects_s3m_unimplemented(player_t * player, int channel)
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