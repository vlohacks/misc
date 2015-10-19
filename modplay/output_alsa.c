/*
 * File:   output_alsa.c
 * Author: vlo
 *
 * Created on July 6, 2013, 2:13 PM
 */

#include "output_alsa.h"
#include <stdio.h>
#include <alsa/asoundlib.h>
#include "player.h"
#include "mixing.h"
snd_pcm_t *playback_handle;
snd_async_handler_t *callback_handle;
snd_pcm_hw_params_t * hw_params;    
snd_pcm_sw_params_t *sw_params;

   
int16_t * output_buffer;
player_t * output_alsa_player;
snd_pcm_uframes_t output_buffer_size = 1024;
snd_pcm_uframes_t period_size = 64;
int output_buffer_pos;

int output_alsa_init(int output_argc, char ** output_argv) 
{
    int err;
    
    output_buffer = (int16_t *)malloc(output_buffer_size * sizeof(int16_t));
    output_buffer_pos = 0;


    
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

    if (snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &output_buffer_size) < 0) {
      fprintf(stderr, "Error setting buffersize.\n");
      exit(1);
    }    
    
    
    if ((err = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &period_size, 0)) < 0) {
        fprintf(stderr, "Error setting period size (%s)\n",
                snd_strerror(err));

        exit(1);
    }    
    fprintf(stderr, "period_size = %i\n", (int)period_size);
    
    



    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        fprintf(stderr, "cannot set parameters (%s)\n",
                snd_strerror(err));
        exit(1);
    }

    snd_pcm_sw_params_alloca (&sw_params);
    
    snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, output_buffer_size - period_size);
    snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, period_size);    
    snd_pcm_sw_params(playback_handle, sw_params);
    
    //snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(playback_handle)) < 0) {
        fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
                snd_strerror(err));
        exit(1);
    }
    
    printf("yay1\n");
    
    //snd_pcm_writei(playback_handle, output_buffer, 2 * period_size);
    printf("yay2\n");
    
    return 0;
}

int output_alsa_cleanup()
{
    snd_pcm_close(playback_handle);
    free(output_buffer);
    return 0;
}

void output_alsa_callback(snd_async_handler_t *pcm_callback) {
        snd_pcm_t * pcm_handle = snd_async_handler_get_pcm(pcm_callback);
        snd_pcm_sframes_t avail;
        int i;
        int j;
        sample_t l, r;
        
        printf("callback\n");
        
        avail = snd_pcm_avail_update(pcm_handle);
        while (avail >= period_size) {
            for (i=0,j=0; i<period_size; i++) {
                player_read(output_alsa_player, &l, &r);
                output_buffer[j++] = sample_to_s16(l);
                output_buffer[j++] = sample_to_s16(r);
            }
            snd_pcm_writei(pcm_handle, output_buffer, period_size);
            avail = snd_pcm_avail_update(pcm_handle);
        }
        
}

int output_alsa_start(player_t * player) {
    printf( "shouldplay2\n");
    output_alsa_player = player;
    player->playing = 1;
    int err = snd_async_add_pcm_handler(&callback_handle, playback_handle, output_alsa_callback, 0);
    if (err != 0)
        fprintf(stderr, "handler failed (%s)\n", snd_strerror(err));
    if (snd_pcm_start(playback_handle) != 0) 
        fprintf(stderr, "start failed\n");
    printf( "shouldplay\n");
    
    return 0;
}

int output_alsa_stop() {
    snd_pcm_drop(playback_handle);
    snd_async_del_handler (callback_handle);
    
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

