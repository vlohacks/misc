#include "output_portaudio.h"
#include "player.h"
#include "output.h"
#include <portaudio.h>
#include <stdio.h>
#include <stdlib.h>


player_t * output_portaudio_player;

PaStream * output_portaudio_stream;


int output_portaudio_init(output_opts_t * output_opts)
{
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "Error initializing PortAudio: %s", Pa_GetErrorText(err));
        exit(1);
    }
        
    err = Pa_OpenDefaultStream( 
        &output_portaudio_stream,
        0, /* no input channels */
        2, /* stereo output */
        paFloat32, /* 32 bit floating point output */
        output_opts->sample_rate,
        output_opts->buffer_size,
        output_portaudio_callback, /* this is your callback function */
        0 );

    if( err != paNoError ) {
        fprintf(stderr, "Error initializing PortAudio Stream: %s", Pa_GetErrorText(err));
        exit(1);
    }
    
    return 0;
}

int output_portaudio_cleanup() 
{
    PaError err;
    err = Pa_Terminate();
    if (err != paNoError) {
        fprintf(stderr, "Error terminating PortAudio: %s", Pa_GetErrorText(err));
        exit(1);
    }

    return 0;
}

int output_portaudio_start(player_t * player)
{
    PaError err;
    output_portaudio_player = player;
    player->playing = 1;
    err = Pa_StartStream(output_portaudio_stream);
    if(err != paNoError) {
        fprintf(stderr, "Error starting PortAudio Stream: %s", Pa_GetErrorText(err));
        exit(1);
    }
    
    return 0;
}

int output_portaudio_stop()
{
    Pa_StopStream(output_portaudio_stream);
    return 0;
}

void output_portaudio_wait()
{
    Pa_Sleep(0);
}

int output_portaudio_callback(
        const void * inputBuffer, 
        void * outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo * timeInfo,
        PaStreamCallbackFlags statusFlags,
        void * userData)
{
    unsigned long i;
    int ret;
    float * out = outputBuffer;
    float mix_l, mix_r;
    
    for (i=0; i < framesPerBuffer; i++) {
        ret = player_read(output_portaudio_player, &mix_l, &mix_r);
        if (ret != 2) {
            output_portaudio_player->playing = 0;
        } else {
            *out++ = mix_l;
            *out++ = mix_r;
        }
    }
    
    return 0;
        
}