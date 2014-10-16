/* 
 * File:   output.h
 * Author: vlo
 *
 * Created on 13. Juli 2013, 17:56
 */

#ifndef OUTPUT_H
#define	OUTPUT_H

#include "player.h"

typedef enum {
    output_driver_portaudio,
    output_driver_raw,
    output_driver_benchmark
} output_drivers_t;

/* TODO? Idee, die output-spezifischen globales in einem union zu verwalten um speichen zu sparen..
struct output_settings {
    output_drivers_t selected_driver;
    union {
        struct portaudio {
            player_t * player;

        };
        struct raw {
            player_t * player;
        };
    };
};
*/

typedef struct {
    output_drivers_t driver;
    int buffer_size;
    int sample_rate;
    char * output_device;
} output_opts_t;

typedef struct {
    output_drivers_t driver;
    int (* init)(output_opts_t * output_opts);
    int (* cleanup)(void);
    int (* start)(player_t * player);
    int (* stop)();
} output_generic_t;

int output_init(output_opts_t * output_opts);
int output_cleanup();
int output_start(player_t * player);
int output_stop();

#endif	/* OUTPUT_H */

