/* 
 * File:   module.h
 * Author: vlo
 * 
 * Internal module structures
 *
 * Created on 29. Juni 2013, 13:22
 */

#ifndef MODULE_H
#define	MODULE_H

#include <stdint.h>
#include "mixing.h"

typedef enum {
    module_type_mod,
    module_type_s3m,
    module_type_mtm,
    module_type_stm,
    module_type_it
} module_type_t;


typedef struct {
    char default_file_extension[8];
    char description[64];
    union {
        struct {
            char signature[8];
        } flags_mod;
        struct {
            uint8_t st2vibrato;            // Vibrato of Scream Tracker 2
            uint8_t st2tempo;              
            uint8_t amigaslides;           // Amiga type volume/pitch slides
            uint8_t amigalimits;           // Amiga period limits for pitch slides
            uint8_t st30volumeslides;      // bugged ST 3.0 Volume slides also performed on tick 0
            uint16_t st3_version;       // used st3 version
            uint8_t default_panning;
            uint8_t mono;                  // mono flag
        } flags_s3m;
        struct {
            uint8_t version_major;
            uint8_t version_minor;
        } flags_stm;
        struct {
            uint8_t stereo;
            uint8_t use_instruments;
            uint8_t linear_slides;
            uint8_t old_effects;
            uint8_t g_effect_link_memory;
            uint16_t created_with;
            uint16_t compatible_with;
        } flags_it;
    };
        
} module_info_t;


// === SAMPLES / INSTRUMENTS ===================================================

typedef struct {
    char        name[29];           // s3m support: extended to 29
    uint32_t    length;
    int8_t      finetune;
    uint16_t    c2spd;              // TODO rename to c_speed - since in IT it is C5Speed
    uint8_t     volume;
    uint8_t     loop_enabled;
    uint32_t    loop_start;
    uint32_t    loop_length;        // TODO is this still used anywhere? if not: remove
    uint32_t    loop_end;
    
    uint32_t    susloop_start;      // IT/XM Substain loop
    uint32_t    susloop_end;

    uint8_t     global_volume;      // IT/XM
    
    uint8_t     vibrato_speed;      // IT/XM per sample vibrato
    uint8_t     vibrato_depth;
    uint8_t     vibrato_waveform;
    uint8_t     vibrato_rate;
    
    uint8_t     pan;                // IT/XM Panning information
} module_sample_header_t;

typedef struct {                    // todo simplify this? makes no sense separating the header infos?
    module_sample_header_t header;
    sample_t * data;
} module_sample_t;

#define MODULE_INSTRUMENT_ENVELOPE_FLAG_ENABLED         (1<<0)
#define MODULE_INSTRUMENT_ENVELOPE_FLAG_LOOP_ENABLED    (1<<1)
#define MODULE_INSTRUMENT_ENVELOPE_FLAG_SUSLOOP_ENABLED (1<<2)
#define MODULE_INSTRUMENT_ENVELOPE_FLAG_ISFILTER        (1<<7)

typedef struct {
    uint8_t     value;
    uint16_t    tick;
} module_instrument_envelope_node_t;

typedef struct {
    uint8_t     flags;
    uint8_t     num_nodes;
    uint8_t     loop_start;
    uint8_t     loop_end;
    uint8_t     susloop_start;
    uint8_t     susloop_end;
    module_instrument_envelope_node_t * nodes;
} module_instrument_envelope_t;

typedef struct {
    char        name[26];
    uint8_t     new_note_action;
    uint8_t     duplicate_check_type;
    uint8_t     duplicate_check_action;
    uint8_t     fadeout;
    int8_t      pitch_pan_separation;
    uint8_t     pitch_pan_center;
    uint8_t     default_pan;
    uint8_t     random_volume_variation;
    uint8_t     random_panning_variation;
    uint8_t     note_sample_table[240];
    module_instrument_envelope_t    volume_envelope;
    module_instrument_envelope_t    pan_envelope;
    module_instrument_envelope_t    pitch_envelope;
} module_instrument_t;



// === PATTERNS ================================================================

typedef struct {
    uint8_t     sample_num;
    int         period_index;
    int8_t      volume;             // s3m support: added for s3m support
    uint8_t     effect_num;
    uint8_t     effect_value;
} module_pattern_data_t;

typedef struct {
    module_pattern_data_t * data;
} module_pattern_row_t;

typedef struct {
    uint16_t num_rows;
    module_pattern_row_t * rows;
} module_pattern_t;


// === MODULE ==================================================================

typedef struct {
    char song_title[21];
    module_info_t module_info;
    module_type_t module_type;
    uint16_t num_channels;
    uint16_t num_samples;
    uint16_t num_instruments;
    uint16_t num_patterns;
    uint16_t num_orders;
    uint8_t orders[256];
    module_sample_t * samples;
    module_instrument_t * instruments;
    module_pattern_t * patterns;
    uint8_t initial_speed;              // s3m support
    uint8_t initial_bpm;
    uint8_t initial_master_volume;
    uint8_t mix_volume;
    uint8_t initial_panning[64];
    uint8_t channel_volume[64];
    uint8_t panning_separation;
    char * song_message;
} module_t;

int module_free(module_t * module);
void module_dump_c(module_t * module);

#endif	/* MODULE_H */

