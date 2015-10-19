#include "output_benchmark.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>

volatile uint64_t output_benchmark_sample_count;
volatile time_t last_time;
volatile int output_benchmark_thread_running;
pthread_t output_benchmark_thread;
volatile player_t * output_benchmark_player;

void * output_benchmark_writer(void * p) {
    sample_t mix_l, mix_r;
    time_t current_time;
    int ret;
    while (output_benchmark_thread_running) {
        current_time = time(0);
        ret = player_read((player_t *)output_benchmark_player, &mix_l, &mix_r);
        if (ret != 2)
            output_benchmark_player->playing = 0;
        output_benchmark_sample_count++;
        if (current_time != last_time) {
            printf("%lu samples/sec\n", output_benchmark_sample_count);
            output_benchmark_sample_count = 0;
            last_time = current_time;
        }
    }
    return 0;
}

int output_benchmark_init(output_opts_t * output_opts)
{
    return 0;
}

int output_benchmark_start(player_t * player) {
    output_benchmark_player = player;
    output_benchmark_thread_running = 1;
    last_time = time(0);
    output_benchmark_sample_count = 0;          
    player->playing = 1;
    
    if (pthread_create(&output_benchmark_thread, 0, output_benchmark_writer, 0)) 
        return 1;
    
    return 0;
    
}

int output_benchmark_stop() {
    output_benchmark_thread_running = 0;
    pthread_join(output_benchmark_thread, 0);
    return 0;
}

int output_benchmark_cleanup() {
    return 0;
}

