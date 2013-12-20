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

typedef enum {
    player_resampling_none = 0,
    player_resampling_linear = 1       
} player_resampling_t;

typedef struct {
    uint8_t sample_num;
    float sample_pos;
    uint8_t sample_delay;
    uint16_t period;
    uint16_t dest_period;
    uint8_t dest_sample_num;
    uint8_t dest_volume;
    uint8_t panning;                            // 0..63;
    float frequency;
    uint8_t volume;
    uint8_t current_effect_num;
    uint8_t current_effect_value;
    uint8_t effect_last_value[16];
    uint8_t effect_last_value_y[16];
    int vibrato_state;
    int tremolo_state;
    uint8_t volume_master;
    uint8_t pattern_loop_position;
    uint8_t pattern_loop_count;
} player_channel_t;


typedef void (*order_callback_t)(module_t *, int current_order, int current_pattern);
typedef void (*row_callback_t)(module_t *, int current_order, int current_pattern, int current_row);
typedef void (*tick_callback_t)(module_t *, int current_order, int current_pattern, int current_row, int current_tick, player_channel_t * channels);


struct player_t;

typedef void (*effect_callback_t)(struct player_t *, int);
typedef void (*newrowaction_callback_t)(struct player_t *, module_pattern_data_t *, int);

struct player_t {
    player_channel_t * channels;                        // channels structs (n-channels elements, alloced when setting module)
    module_t * module;                                  // module to play
    
    effect_callback_t * effect_map;                     // effects map (module format specific)
    newrowaction_callback_t newrow_action;              // function which does all stuff when a new row is called (module format specific)

    order_callback_t order_callback;                    // callback pointer which gets called every order change
    row_callback_t row_callback;                        // callback pointer which gets called every row (patter view..)
    tick_callback_t tick_callback;                      // callback pointer which gets called every tick (effects, volumebars)

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
    float sample_rate;                                  // samplerate, normally matches output device"s rate

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
    int playing;
};

typedef struct player_t player_t;

/* Protoypes
 */
player_t * player_init(const float samplerate, const player_resampling_t resampling);
void player_free(player_t * player);
void player_set_protracker_strict_mode(player_t * player, int enabled);
void player_set_module(player_t * player, module_t * module);
int player_read(player_t * player, float * mix_l, float * mix_r);
void player_register_tick_callback(player_t * player, tick_callback_t func);
void player_register_row_callback(player_t * player, row_callback_t func);
void player_register_order_callback(player_t * player, order_callback_t func);

void player_init_channels(player_t * player);
void player_init_defaults(player_t * player);
float player_calc_tick_duration(const uint16_t bpm, const float sample_rate);
void player_channel_set_frequency(player_t * player, const uint16_t period, const int channel_num);
float player_channel_fetch_sample(player_t * player,  const int channel_num) ;

#endif	/* PLAYER_H */

