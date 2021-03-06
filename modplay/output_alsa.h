/* 
 * File:   output_alsa.h
 * Author: vlo
 *
 * Created on July 6, 2013, 2:40 PM
 */

#ifndef OUTPUT_ALSA_H
#define	OUTPUT_ALSA_H

#include "player.h"

int output_alsa_init(int output_argc, char ** output_argv);
int output_alsa_cleanup();
int output_alsa_start(player_t * player);
int output_alsa_stop();

int output_alsa_write(float l, float r);

#endif	/* OUTPUT_ALSA_H */

