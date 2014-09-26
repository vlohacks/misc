/* 
 * File:   output.h
 * Author: vlo
 *
 * Created on 13. Juli 2013, 17:56
 */

#ifndef OUTPUT_H
#define	OUTPUT_H

typedef enum {
    output_driver_portaudio,
    output_driver_raw
} output_drivers_t;

typedef struct {
    output_drivers_t driver;
    int buffer_size;
    int sample_rate;
    char * output_device;
} output_opts_t;

#endif	/* OUTPUT_H */

