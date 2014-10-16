/* 
 * File:   output_benchmark.h
 * Author: vlo
 * 
 * Output driver for measuring the performance of the mod_player. 
 * Does not acutally make noise :-)
 *
 * Created on 16. Oktober 2014, 21:45
 */


#ifndef OUTPUT_BENCHMARK_H
#define	OUTPUT_BENCHMARK_H

#include "player.h"
#include "output.h"



int output_benchmark_init(output_opts_t * output_opts);
int output_benchmark_start(player_t * player);
int output_benchmark_stop();
int output_benchmark_cleanup();


#endif	/* OUTPUT_BENCHMARK_H */

