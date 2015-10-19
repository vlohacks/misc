/* 
 * File:   player.c
 * Author: vlo
 * 
 * Player engine
 *
 * Created on 29. Juni 2013, 13:57
 */

#include <stdlib.h>
#include <stdio.h>
#include "player.h"
#include "defs_mod.h"
#include "defs_s3m.h"
#include "defs_stm.h"
#include "effects_mod.h"
#include "effects_s3m.h"
#include "effects_stm.h"
#include "math.h"
#include "mixing.h"


player_t * player_init(const uint32_t sample_rate, const player_resampling_t resampling) 
{
    player_t * player = (player_t *)malloc(sizeof(player_t));
    
    player->sample_rate = sample_rate;
    player->resampling = resampling;
    player->channels = 0;
    player->effect_map = 0;
    player->paula_freq_index = PAL;
    player->loop_module = 0;
    
    player->tick_callback = 0;
    player->row_callback = 0;
    player->order_callback = 0;
    player->channel_sample_callback = 0;

    player->loop_pattern = -1;
    player->solo_channel = -1;
    
    player_set_protracker_strict_mode(player, 0);

    return player;
}

void player_free(player_t * player)
{
    if (player->effect_map)
        free(player->effect_map);
    
    if (player->channels)
        free(player->channels);    
    
    free(player);
}

void player_set_protracker_strict_mode(player_t * player, int enabled)
{
    /* protracker_strict_mode for non-protracker modules makes no sense */
    /*
    if (player->module->module_type != module_type_mod) {
        player->protracker_strict_mode = 0;
        return;
    }
    */
    
    player->protracker_strict_mode = enabled;
    if (enabled) {
        player->period_bottom = 113;
        player->period_top = 856;
    } else {
        player->period_bottom = defs_mod_periods[defs_mod_num_periods - 1];
        player->period_top = defs_mod_periods[0];
    }
}


void player_set_module(player_t * player, module_t * module) 
{
    player->module = module;    

    player_init_channels(player);
    player_init_defaults(player);
    
    player->current_order = 0;
    player->next_order = 0;
    player->current_row = 0;
    player->next_row = player->current_row++;
    
    player->current_pattern = player->loop_pattern >= 0
        ? player->loop_pattern
        : player->module->orders[player->current_order];
    
    player->tick_pos = 0;
    player->tick_duration = player_calc_tick_duration(player->bpm, player->sample_rate);
    player->do_break = 1;       // to initialize state 
    player->current_tick = player->module->initial_speed;
    player->pattern_delay = 0;
    
    if (player->effect_map) {
        free(player->effect_map);
        player->effect_map = 0;
    }
    
    // map effects dependent on module type
    switch (module->module_type) {
        case module_type_mtm:
        case module_type_mod: 
            player->effect_map = effects_mod_init(); 
            player->newrow_action = (newrowaction_callback_t)effects_mod_newrowaction;
            player->period_table = defs_mod_periods;
            break;
            
        case module_type_s3m:
            player->effect_map = effects_s3m_init();
            player->newrow_action = (newrowaction_callback_t)effects_s3m_newrowaction;
            player->period_table = defs_s3m_periods;
            player->period_top = defs_s3m_periods[0];
            player->period_bottom = defs_s3m_periods[defs_s3m_num_periods - 1];
            break;
            
        case module_type_stm:
            player->effect_map = effects_stm_init();
            player->newrow_action = (newrowaction_callback_t)effects_stm_newrowaction;
            player->period_table = defs_stm_periods;
            player->period_top = defs_stm_periods[0];
            player->period_bottom = defs_stm_periods[defs_stm_num_periods - 1];
            break;
    }
}

void player_init_channels(player_t * player) 
{
    int i, j;
    
    if (player->channels)
        free(player->channels);
    
    player->channels = (player_channel_t *)malloc(sizeof(player_channel_t) * player->module->num_channels);
    
    for (i = 0; i < player->module->num_channels; i++) {
        
        player->channels[i].period = 0;
        player->channels[i].period_index = -1;
        player->channels[i].sample_num = 0;
        player->channels[i].sample_pos = 0;
        player->channels[i].volume = 64;
        player->channels[i].volume_master = 64;
        for (j = 0; j < 26; j++) {
                player->channels[i].effect_last_value[j] = 0;
                player->channels[i].effect_last_value_y[j] = 0;
        }
        player->channels[i].effect_num = 0;
        player->channels[i].effect_value = 0;
        
        player->channels[i].vibrato_state = 0;
        player->channels[i].tremolo_state = 0;
        player->channels[i].tremor_state = 0;
        player->channels[i].vibrato_waveform = 0;
        player->channels[i].tremolo_waveform = 0;
        player->channels[i].pattern_loop_position = 0;
        player->channels[i].pattern_loop_count = 0;     
        player->channels[i].sample_delay = 0;
        
        // default pannings
        player->channels[i].panning = player->module->initial_panning[i];
    }
    
}

/* render next samples to mix_l and mix_r, update the player state
 */
int player_read(player_t * player, sample_t * out_l, sample_t * out_r)
{
    int k;
    sample_mac_t mix_l, mix_r;
    
    // reaching new tick
    if (player->tick_pos <= 0) {
        
        // last tick reached, advance to next row
        if (player->current_tick == player->speed) {
            // if there is a pattern delay, don't advance to the next row
            if (player->pattern_delay) {
                player->pattern_delay--;
                player->pattern_delay_active = 1;
            } else {
                player->current_row = player->next_row;
                player->next_row++;
                player->pattern_delay_active = 0;
            }
            
            player->current_tick = 0;
            
            // advance to next order if last row played or break upcoming
            if ((player->current_row >= 64) || (player->do_break)) {
                
                player->current_order = player->next_order;
                player->next_order++;
                player->do_break = 0;
                
                player->pattern_delay_active = 0;
                
                // only if regular pattern end (no break)
                if ((player->current_row) >= 64) {
                    player->current_row = 0;
                    player->next_row = 1;
                }

                // loop if looping enabled 
                if (player->next_order >= player->module->num_orders) {
                    if (player->loop_module) 
                        player->next_order = 0;
                }
                
                // end of song reached...
                if (player->current_order >= player->module->num_orders)
                    return 0;

                // lookup pattern to play in order list
                if (player->loop_pattern >= 0)
                    player->current_pattern = player->loop_pattern;
                else 
                    player->current_pattern = player->module->orders[player->current_order];

                if (player->order_callback)
                    //(player->order_callback)(player, player->current_order, player->current_pattern);
                    (player->order_callback)(player, player->callback_user_ptr);

            }

            // maintain row callback
            if (player->row_callback)
                //(player->row_callback)(player, player->current_order, player->current_pattern, player->current_row);
                (player->row_callback)(player, player->callback_user_ptr);

            // fetch new pattern data from module
            for (k = 0; k < player->module->num_channels; k++) {
                module_pattern_data_t * current_data = &(player->module->patterns[player->current_pattern].rows[player->current_row].data[k]);

                player->channels[k].effect_num = current_data->effect_num;
                player->channels[k].effect_value = current_data->effect_value;
                
                player->newrow_action(player, current_data, k);
            }

        }

        // maintain effects
        for (k=0; k < player->module->num_channels; k++) {
            /*
            if (player->current_tick == 0) {
                if (player->channels[k].effect_value)
                    player->channels[k].effect_last_value[player->channels[k].effect_num] = player->channels[k].effect_value;
            }
            */
            if ((player->effect_map)[player->channels[k].effect_num])
                (player->effect_map)[player->channels[k].effect_num](player, k);
        }
        
        // go for next tick
        player->current_tick++;
        player->tick_pos = player->tick_duration;

        // maintain tick callback
        if (player->tick_callback)
            //(player->tick_callback)(player, player->current_order, player->current_pattern, player->current_row, player->current_tick, player->channels);
            (player->tick_callback)(player, player->callback_user_ptr);
        

    }

    // mixing
    mix_l = mix_r = 0;
    sample_mac_t cl, cr;
    
    for (k = 0; k < player->module->num_channels; k++) {
        if ((player->solo_channel >= 0) && (k != player->solo_channel))
            continue;
        
        sample_mac_t s = player_channel_fetch_sample(player, k);

        // Performance Tuning: no need to do anything with 0-samples
        if (s) {
            s *= player->channels[k].volume_master;
            s *= player->channels[k].volume;
            s /= (64 * 64); 
            
            cr = (s * player->channels[k].panning) / 256;
            cl = (s * (255 - player->channels[k].panning)) / 256;
        
            mix_l += cl;
            mix_r += cr;
        }
        
        if (player->channel_sample_callback) 
            (player->channel_sample_callback)(player, player->callback_user_ptr);
        
    }
    
    mix_l /= ((sample_mac_t)player->module->num_channels);
    mix_r /= ((sample_mac_t)player->module->num_channels);
    *out_l = (sample_t)mix_l;
    *out_r = (sample_t)mix_r;

    player->tick_pos--;
    
    return 2;
}

void player_register_tick_callback(player_t * player, player_callback_t func)
{
    player->tick_callback = func;
}

void player_register_row_callback(player_t * player, player_callback_t func)
{
    player->row_callback = func;
}

void player_register_order_callback(player_t * player, player_callback_t func)
{
    player->order_callback = func;
}

void player_register_channel_sample_callback(player_t * player, player_callback_t func, uint32_t callback_mask)
{
    player->channel_sample_callback_mask = callback_mask;
    player->channel_sample_callback = func;
}

void player_register_callback_user_ptr(player_t * player, void * ptr) 
{
    player->callback_user_ptr = ptr;
}

void player_init_defaults(player_t * player) 
{
    player->bpm = player->module->initial_bpm;
    player->speed = player->module->initial_speed;
}

float player_calc_tick_duration(const uint16_t bpm, const uint32_t sample_rate) 
{
    return ((((float)sample_rate / ((float)bpm / 60.0f)) / 4.0f) / 6.0f);
    //return (((sample_rate / (bpm / 60)) / 4) / 6);
}



void player_channel_set_frequency(player_t * player, const uint16_t period, const int channel_num)
{
    const float x = 1.007246412;
    
    player_channel_t * channel = &(player->channels[channel_num]);
    module_sample_t * sample = &(player->module->samples[channel->sample_num - 1]);
    
    int8_t finetune;
    //uint16_t tuned_period;
    
    /* no period? bail out */
    if (!period)
        return;
    
    switch (player->module->module_type) {
        case module_type_mod:
        case module_type_mtm:
            // unusual: finetune is a signed nibble
            finetune = (sample->header.finetune >= 8 
                    ? -(16 - sample->header.finetune) 
                    : sample->header.finetune);

            channel->frequency = defs_mod_paulafreq[player->paula_freq_index] / ((float)period * 2.0f);

            // TODO: will there be a day when we can get rid of libm here?
            if (finetune)
                channel->frequency *= pow(x, finetune);

            break;
        
        case module_type_stm:
        case module_type_s3m:
            channel->frequency = 14317056L / period;
            break;
    }
    
    // Performance tuning: do this calculation only once rather than every sample
    channel->sample_step = ((float)channel->frequency / (float)player->sample_rate);
    
}

sample_t player_channel_fetch_sample(player_t * player,  const int channel_num) 
{
    sample_mac_t s, s2;
    player_channel_t * channel = &(player->channels[channel_num]);
    
    // avoid reading past num_samples (some MODs access sample slots beyond num_samples, for examples the UT soundtrack )
    if (channel->sample_num > player->module->num_samples)
        return SAMPLE_T_ZERO;
    
    module_sample_t * sample = &(player->module->samples[channel->sample_num - 1]);
    
    // no sample, no sound... 
    if (channel->sample_num == 0)
        return SAMPLE_T_ZERO;
    
    // trying to play a empty sample slot... play silence instead of segfault :)
    //if (player->module->samples[sample_index].data == 0)
    if (sample->data == 0)
        return SAMPLE_T_ZERO;
    
    // maintain looping
    if (sample->header.loop_enabled) {
        while (channel->sample_pos >= (float)(sample->header.loop_end)) {
            channel->sample_pos -= (float)sample->header.loop_length;
        }
    } else {
        if (channel->sample_pos >= (float)sample->header.length)
            return SAMPLE_T_ZERO;
    }
    
    // fetch sample 
    s = sample->data[(uint16_t)(channel->sample_pos)];
    
    if (player->resampling == player_resampling_linear) {
        // do linear interpolation
        if (sample->header.loop_enabled) {
            // looping sample will interpolate to loop start
            if (channel->sample_pos >= (float)(sample->header.loop_end)) 
                s2 = sample->data[sample->header.loop_start];
            else 
                s2 = sample->data[(uint16_t)(channel->sample_pos) + 1];
        } else {
            if (channel->sample_pos < (sample->header.length - 1)) 
                s2 = sample->data[(uint16_t)(channel->sample_pos) + 1];
            else
                s2 = s;
        }
        // conversion to int will remove the fractional part of the sample_pos
        s += (s2 - s) * (channel->sample_pos - (float)((int)channel->sample_pos));  
    }
    
    // advance sample position
    channel->sample_pos += channel->sample_step;
    //channel->sample_pos += ((float)channel->frequency / (float)player->sample_rate);

    return (sample_t)s;
}
