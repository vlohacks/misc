/*
 * File:   output_alsa.c
 * Author: vlo
 *
 * Created on July 6, 2013, 2:13 PM
 */

#include "output_alsa.h"
#include <stdio.h>
#include <alsa/asoundlib.h>

snd_pcm_t *playback_handle;

int16_t * output_buffer;
int output_buffer_size = 1024;
int output_buffer_pos;

int output_alsa_init(int output_argc, char ** output_argv) 
{
    int err;
    
    output_buffer = (int16_t *)malloc(output_buffer_size * sizeof(int16_t));
    output_buffer_pos = 0;

    snd_pcm_hw_params_t * hw_params;    
    
    char * device = "default";

    if ((err = snd_pcm_open(&playback_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "cannot open audio device %s (%s)\n",
                device,
                snd_strerror(err));
        exit(1);
    }

   
    snd_pcm_hw_params_alloca(&hw_params);

    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "cannot set access type (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "cannot set sample format (%s)\n",
                snd_strerror(err));
        exit(1);
    }

  
    unsigned int rate = 44100;
    int dir = 0;
    unsigned int periods = 2;
    //snd_pcm_uframes_t periodsize = 8192;
    
    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0)) < 0) {
        fprintf(stderr, "cannot set sample rate (%s)\n",
                snd_strerror(err));
        exit(1);
    }
    fprintf(stderr, "rate = %i\n", rate);

    if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2)) < 0) {
        fprintf(stderr, "cannot set channel count (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    
    if ((err = snd_pcm_hw_params_set_periods_near(playback_handle, hw_params, &periods, 0)) < 0) {
        fprintf(stderr, "Error setting periods (%s)\n",
                snd_strerror(err));

        exit(1);
    }    
    fprintf(stderr, "periods = %i\n", periods);
        
    if (snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, (output_buffer_size * periods) >> 2) < 0) {
      fprintf(stderr, "Error setting buffersize.\n");
      exit(1);
    }


    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot set parameters (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    //snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(playback_handle)) < 0) {
        fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
                snd_strerror(err));
        exit(1);
    }
    
    return 0;
}

int output_alsa_cleanup()
{
    snd_pcm_close(playback_handle);
    free(output_buffer);
    return 0;
}

int output_alsa_write(float l, float r) 
{
    int err;
    //output_buffer_pos = 0;
    output_buffer[output_buffer_pos] = (int16_t)(l * 32767);
    output_buffer[output_buffer_pos + 1] = (int16_t)(r * 32767);
    //snd_pcm_writei(playback_handle, output_buffer, 2);
    //snd_pcm_prepare(playback_handle);
    //return 2;
    output_buffer_pos += 2;
    if (output_buffer_pos > (output_buffer_size) ) {
        if ((err = snd_pcm_writei(playback_handle, output_buffer, output_buffer_size / 2)) != (output_buffer_size / 2)) {
            fprintf (stderr, "write to audio interface failed (%s)\n",
                    snd_strerror (err));
            exit (1);
        }
        //snd_pcm_prepare(playback_handle);
        output_buffer_pos = 0;
    }
    return 2;
}

