/* 
 * File:   output_raw.h
 * Author: vlo
 *
 * Created on 26. September 2014, 21:47
 */

#ifndef OUTPUT_RAW_H
#define	OUTPUT_RAW_H

int output_raw_init(output_opts_t * output_opts);
int output_raw_cleanup();
int output_raw_start(player_t * player);
int output_raw_stop();



#endif	/* OUTPUT_RAW_H */

