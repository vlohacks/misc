/* 
 * File:   output_portaudio.h
 * Author: vlo
 *
 * Created on 6. Juli 2013, 22:39
 */

#ifndef OUTPUT_PORTAUDIO_H
#define	OUTPUT_PORTAUDIO_H

#include "player.h"
#include "output.h"
#include <portaudio.h>

int output_portaudio_init(output_opts_t * output_opts);
int output_portaudio_cleanup();
int output_portaudio_start(player_t * player);
int output_portaudio_stop();
void output_portaudio_wait();

int output_portaudio_callback(
        const void * inputBuffer, 
        void * outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo * timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData);

#endif	/* OUTPUT_PORTAUDIO_H */

