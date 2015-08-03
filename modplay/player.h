/* 
 * File:   player.h
 * Author: vlo
 * 
 * Player engine
 *
 * Created on 29. Juni 2013, 15:01
 */

#ifndef PLAYER_H
#define	PLAYER_H

#include "module.h"
#include "defs_mod.h"
#include "defs_s3m.h"
#include "mixing.h"

typedef enum {
    player_resampling_none = 0,
    player_resampling_linear = 1       
} player_resampling_t;

typedef struct {
    uint8_t sample_num;
    float sample_pos;
    float sample_step;
    
    int period_index;
    uint16_t period;
    uint32_t frequency;
    

    uint8_t sample_delay;
    uint16_t dest_period;                       // these are used for tone portamento and not delay
    uint8_t dest_sample_num;                    
    int8_t dest_volume;
    uint8_t panning;                            

    int8_t volume;                              
    int8_t volume_master;                       // used for effects like tremolo    
    
    uint8_t effect_num;                         // currently played effect on track
    uint8_t effect_value;                       // currently player effect parameters on track
    uint8_t effect_last_value[26];              // for effects remembering their parameters 
    uint8_t effect_last_value_y[26];
    
    int vibrato_state;                          // status of effects
    int tremolo_state;
    int tremor_state;
    
    uint8_t vibrato_waveform;
    uint8_t tremolo_waveform;

    uint8_t pattern_loop_position;
    uint8_t pattern_loop_count;
} player_channel_t;




struct player_t;
/*
typedef void (*order_callback_t)(struct player_t *, int current_order, int current_pattern);
typedef void (*row_callback_t)(struct player_t *, int current_order, int current_pattern, int current_row);
typedef void (*tick_callback_t)(struct player_t *, int current_order, int current_pattern, int current_row, int current_tick, player_channel_t * channels);
typedef void (*channel_sample_callback_t)(sample_t l, sample_t r, sample_t peak_l, sample_t peak_r, int channel);
 */
typedef void (*player_callback_t)(struct player_t *, void * user_ptr);


typedef void (*effect_callback_t)(struct player_t *, int);
typedef void (*newrowaction_callback_t)(struct player_t *, module_pattern_data_t *, int);

struct player_t {
    player_channel_t * channels;                        // channels structs (n-channels elements, alloced when setting module)
    module_t * module;                                  // module to play
    
    effect_callback_t * effect_map;                     // effects map (module format specific)
    newrowaction_callback_t newrow_action;              // function which does all stuff when a new row is called (module format specific)

    void * callback_user_ptr;                           // holds ptr to user data sent back in callbacks
    /*
    order_callback_t order_callback;                    // callback pointer which gets called every order change
    row_callback_t row_callback;                        // callback pointer which gets called every row (patter view..)
    tick_callback_t tick_callback;                      // callback pointer which gets called every tick (effects, volumebars)
    channel_sample_callback_t channel_sample_callback;  // callback pointer which gets called every sample & sample_callback_mask
    */
    player_callback_t order_callback;                   // callback pointer which gets called every order change
    player_callback_t row_callback;                     // callback pointer which gets called every row (patter view..)
    player_callback_t tick_callback;                    // callback pointer which gets called every tick (effects, volumebars)
    player_callback_t channel_sample_callback;          // callback pointer which gets called every sample & sample_callback_mask
    
    uint32_t channel_sample_callback_mask;

    const uint16_t * period_table;                            // will be set when initializing the player with an module.. different formats have different tables
    
    player_resampling_t resampling;                     // sample intermpolation method: linear, none...
    defs_mod_paulafreq_index_t paula_freq_index;        // ntsc or pal
    
    int protracker_strict_mode;                         // disable panning etc.
    
    uint16_t period_top;                                // the minimum legal period, set dependent on protracker strict mode
    uint16_t period_bottom;                             // the maximum legal period
    
    uint8_t speed;                                      // current speed setting (ticks per row)
    uint16_t bpm;                                       // current bpm setting

    float tick_pos;                                     // current position in tick (internal)
    float tick_duration;                                // duration of one tick (gets calculated when set speed effect occurs)
    uint32_t sample_rate;                               // samplerate, normally matches output device"s rate

    int current_tick;
    int current_pattern;
    int current_order;
    int current_row;

    int do_break;
    int next_order;
    int next_row;
    
    int pattern_delay;
    int pattern_delay_active;
    
    int loop_module;
    int loop_pattern;
    int solo_channel;
    
    int playing;
};

typedef struct player_t player_t;

/* Protoypes
 */
player_t * player_init(const uint32_t samplerate, const player_resampling_t resampling);
void player_free(player_t * player);
void player_set_protracker_strict_mode(player_t * player, int enabled);
void player_set_module(player_t * player, module_t * module);
int player_read(player_t * player, sample_t * out_l, sample_t * out_r);

void player_register_tick_callback(player_t * player, player_callback_t func);
void player_register_row_callback(player_t * player, player_callback_t func);
void player_register_order_callback(player_t * player, player_callback_t func);
void player_register_channel_sample_callback(player_t * player, player_callback_t func, uint32_t callback_mask);
void player_register_callback_user_ptr(player_t * player, void * ptr);

void player_init_channels(player_t * player);
void player_init_defaults(player_t * player);
float player_calc_tick_duration(const uint16_t bpm, const uint32_t sample_rate);
void player_channel_set_frequency(player_t * player, const uint16_t period, const int channel_num);
sample_t player_channel_fetch_sample(player_t * player,  const int channel_num) ;

#endif	/* PLAYER_H */

