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
    module_type_mod
} module_type_t;

typedef struct {
    char default_file_extension[8];
    char description[64];
    char signature[8];
} module_info_t;

typedef struct {
    char name[23];
    uint32_t length;
    int8_t finetune;
    uint8_t volume;
    uint32_t loop_start;
    uint32_t loop_length;
} module_sample_header_t;

typedef struct {
    module_sample_header_t header;
    int8_t * data;
} module_sample_t;

typedef struct {
    uint8_t sample_num;
    uint16_t period;
    int period_index;
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
    uint8_t orders[128];
    module_sample_t * samples;
    module_pattern_t * patterns;
} module_t;


int module_free(module_t * module);

#endif	/* MODULE_H */

