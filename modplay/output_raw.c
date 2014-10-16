#include <pthread.h>
#include <stdio.h>
#include "output.h"
#include "player.h"
#include "mixing.h"


char * output_raw_filename;
volatile int thread_running;
pthread_t writer_thread;
volatile player_t * output_raw_player;
volatile FILE * output_raw_file;

void * output_raw_writer(void * p) {
    sample_t mix_l, mix_r;
    int ret;
    while (thread_running) {
        ret = player_read(output_raw_player, &mix_l, &mix_r);
        if (ret != 2)
            output_raw_player->playing = 0;
        fwrite(&mix_l, sizeof(sample_t), 1, output_raw_file);
        fwrite(&mix_r, sizeof(sample_t), 1, output_raw_file);
    }
    return 0;
}

int output_raw_init(output_opts_t * output_opts)
{
    output_raw_file = 0;
    output_raw_filename = output_opts->output_device;
    return 0;
}

int output_raw_start(player_t * player) {
    output_raw_player = player;
    thread_running = 1;
    player->playing = 1;
    output_raw_file = fopen(output_raw_filename, "wb");
    if (output_raw_file == 0) 
        return 1;
    
    if (pthread_create(&writer_thread, 0, output_raw_writer, 0)) 
        return 1;
    
    return 0;
    
}

int output_raw_stop() {
    thread_running = 0;
    pthread_join(writer_thread, 0);
    if (output_raw_file)
        fclose(output_raw_file);
    output_raw_file = 0;
    return 0;
}

int output_raw_cleanup() {
    return 0;
}



