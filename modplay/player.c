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
#include "effects_mod.h"
#include "effects_s3m.h"
#include "math.h"

player_t * player_init(const float sample_rate, const player_resampling_t resampling) 
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
        for (j = 0; j < 32; j++)
                player->channels[i].effect_last_value[j] = 0;
        player->channels[i].current_effect_num = 0;
        player->channels[i].current_effect_value = 0;
        player->channels[i].vibrato_state = 0;
        player->channels[i].tremolo_state = 0;
        player->channels[i].tremor_state = 0;
        player->channels[i].pattern_loop_position = 0;
        player->channels[i].pattern_loop_count = 0;     
        player->channels[i].sample_delay = 0;
        
        // default pannings
        player->channels[i].panning = player->module->initial_panning[i];
    }
    
}

/* render next samples to mix_l and mix_r, update the player state
 */
int player_read(player_t * player, float * mix_l, float * mix_r)
{
    int k;
    static uint32_t samplectr;
    
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
                    (player->order_callback)(player, player->current_order, player->current_pattern);

            }

            // maintain row callback
            if (player->row_callback)
                (player->row_callback)(player, player->current_order, player->current_pattern, player->current_row);

            // fetch new pattern data from module
            for (k = 0; k < player->module->num_channels; k++) {
                module_pattern_data_t * current_data = &(player->module->patterns[player->current_pattern].rows[player->current_row].data[k]);
                
                player->newrow_action(player, current_data, k);
                
                player->channels[k].current_effect_num = current_data->effect_num;
                player->channels[k].current_effect_value = current_data->effect_value;
            }

        }

        // maintain effects
        for (k=0; k < player->module->num_channels; k++) {
            if (player->current_tick == 0) {
                if (player->channels[k].current_effect_value)
                    player->channels[k].effect_last_value[player->channels[k].current_effect_num] = player->channels[k].current_effect_value;
            }
            if ((player->effect_map)[player->channels[k].current_effect_num])
                (player->effect_map)[player->channels[k].current_effect_num](player, k);
        }
        
        // go for next tick
        player->current_tick++;
        player->tick_pos = player->tick_duration;

        // maintain tick callback
        if (player->tick_callback)
            (player->tick_callback)(player, player->current_order, player->current_pattern, player->current_row, player->current_tick, player->channels);
        

    }

    // mixing
    *mix_l = *mix_r = 0;
    
    for (k = 0; k < player->module->num_channels; k++) {
        if ((player->solo_channel >= 0) && (k != player->solo_channel))
            continue;
        
        float panning = ((float)player->channels[k].panning) / 255.0f;
        float s = player_channel_fetch_sample(player, k) * ((float)player->channels[k].volume_master / 64.0f);
        float cl = (s * panning);
        float cr = (s * (1.0f - panning));
        float tmp;
        
        *mix_l += cl;
        *mix_r += cr;
        
        if (player->channel_sample_callback) {
            tmp = cl < 0 ? cl * -1 : cl;
            if (tmp > player->channels[k].peak_sample[0])
                player->channels[k].peak_sample[0] = tmp;
            
            tmp = cr < 0 ? cr * -1 : cr;
            if (tmp > player->channels[k].peak_sample[1])
                player->channels[k].peak_sample[1] = tmp;
            
            if ((samplectr & player->channel_sample_callback_mask) == 0) {
                (player->channel_sample_callback)(cl, cr, player->channels[k].peak_sample[0], player->channels[k].peak_sample[1], k);
                player->channels[k].peak_sample[0] = 0;
                player->channels[k].peak_sample[1] = 0;
            }
        }
        
    }
    
    samplectr++;
    
    *mix_l /= ((float)player->module->num_channels / 2.0f);
    *mix_r /= ((float)player->module->num_channels / 2.0f);

#if 0    
    if (player->protracker_strict_mode) {
        /* in pt strict mode without panning, channels are mapped strictly
         * RLLR RLLR... therefore we have exactly 50% on each channel 
         * and we only need to normalize 50%
         */
        *mix_l /= ((float)player->module->num_channels / 2.0f);
        *mix_r /= ((float)player->module->num_channels / 2.0f);
    } else {
        *mix_l /= ((float)player->module->num_channels);
        *mix_r /= ((float)player->module->num_channels);
    }
#endif

    player->tick_pos--;
    
    return 2;
}

void player_register_tick_callback(player_t * player, tick_callback_t func)
{
    player->tick_callback = func;
}

void player_register_row_callback(player_t * player, row_callback_t func)
{
    player->row_callback = func;
}

void player_register_order_callback(player_t * player, order_callback_t func)
{
    player->order_callback = func;
}

void player_register_channel_sample_callback(player_t * player, channel_sample_callback_t func, uint32_t callback_mask)
{
    player->channel_sample_callback_mask = callback_mask;
    player->channel_sample_callback = func;
}

void player_init_defaults(player_t * player) 
{
    player->bpm = player->module->initial_bpm;
    player->speed = player->module->initial_speed;
}

float player_calc_tick_duration(const uint16_t bpm, const float sample_rate) 
{
    return (((sample_rate / ((float)bpm / 60.0f)) / 4.0f) / 6.0f);
}



void player_channel_set_frequency(player_t * player, const uint16_t period, const int channel_num)
{
    const float x = 1.007246412;
    
    player_channel_t * channel = &(player->channels[channel_num]);
    module_sample_t * sample = &(player->module->samples[channel->sample_num - 1]);
    
    int8_t finetune;
    //uint16_t tuned_period;
    
    switch (player->module->module_type) {
        case module_type_mod:
            // unusual: finetune is a signed nibble
            finetune = (sample->header.finetune >= 8 
                    ? -(16 - sample->header.finetune) 
                    : sample->header.finetune);

            channel->frequency = defs_mod_paulafreq[player->paula_freq_index] / ((float)period * 2.0f);

            if (finetune)
                channel->frequency *= pow(x, finetune);

            break;
        
        case module_type_s3m:
            channel->frequency = 14317056L / period;
            break;
    }
    
}

float player_channel_fetch_sample(player_t * player,  const int channel_num) 
{
    float s, s2;
    
    player_channel_t * channel = &(player->channels[channel_num]);
    int sample_index = channel->sample_num - 1;

    // no sample, no sound... 
    if (channel->sample_num == 0)
        return 0.0f;

    // trying to play a empty sample slot... play silence instead of segfault :)
    if (player->module->samples[sample_index].data == 0)
        return 0.0f;
    
    
    // maintain looping
    if (player->module->samples[sample_index].header.loop_enabled) {
        while (channel->sample_pos >= (float)(player->module->samples[sample_index].header.loop_end)) {
            channel->sample_pos -= (float)player->module->samples[sample_index].header.loop_length;
        }
    } else {
        if (channel->sample_pos >= (float)player->module->samples[sample_index].header.length)
            return 0.0f;
    }
    
    // fetch sample and convert to float
    s = (float)player->module->samples[sample_index].data[(uint16_t)(channel->sample_pos)] / 128.0f;

    if (player->resampling == player_resampling_linear) {
        // do linear interpolation
        if (player->module->samples[sample_index].header.loop_length > 2) {
            // looping sample will interpolate to loop start
            if (channel->sample_pos >= (float)(player->module->samples[sample_index].header.loop_length + player->module->samples[sample_index].header.loop_start)) 
                s2 = (float)player->module->samples[sample_index].data[player->module->samples[sample_index].header.loop_start] / 128.0f;
            else 
                s2 = (float)player->module->samples[sample_index].data[(uint16_t)(channel->sample_pos) + 1] / 128.0f;
        } else {
            if (channel->sample_pos < (player->module->samples[sample_index].header.length - 1)) 
                s2 = (float)player->module->samples[sample_index].data[(uint16_t)(channel->sample_pos) + 1] / 128.0f;
            else
                s2 = s;
        }
        s += (s2 - s) * (channel->sample_pos - (float)((int)channel->sample_pos));  
    }

    // maintain channel volume
    s *= ((float)channel->volume / 64.0f);
    
    // advance sample position
    channel->sample_pos += (channel->frequency / player->sample_rate);

    return s;
}