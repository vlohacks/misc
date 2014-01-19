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

typedef enum {
    module_type_mod,
    module_type_s3m,
    module_type_mtm
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
    };
        
} module_info_t;

typedef struct {
    char name[29];              // s3m support: extended to 29
    uint32_t length;
    int8_t finetune;
    uint16_t c2spd;
    uint8_t volume;
    uint8_t loop_enabled;
    uint32_t loop_start;
    uint32_t loop_length;
    uint32_t loop_end;                                     
} module_sample_header_t;

typedef struct {
    module_sample_header_t header;
    int8_t * data;
} module_sample_t;

typedef struct {
    uint8_t sample_num;
    //uint16_t period;
    int period_index;
    int8_t volume;             // s3m support: added for s3m support
    uint8_t effect_num;
    uint8_t effect_value;
} module_pattern_data_t;

typedef struct {
    module_pattern_data_t * data;
} module_pattern_row_t;

typedef struct {
    uint16_t num_rows;
    module_pattern_row_t * rows;
} module_pattern_t;

typedef struct {
    char song_title[21];
    module_info_t module_info;
    module_type_t module_type;
    uint16_t num_channels;
    uint16_t num_samples;
    uint16_t num_patterns;
    uint16_t num_orders;
    uint8_t orders[256];
    module_sample_t * samples;
    module_pattern_t * patterns;
    uint8_t initial_speed;              // s3m support
    uint8_t initial_bpm;
    uint8_t initial_master_volume;
    uint8_t initial_panning[32];
    char * song_message;
} module_t;


int module_free(module_t * module);

#endif	/* MODULE_H */

