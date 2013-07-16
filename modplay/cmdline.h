/* 
 * File:   cmdline.h
 * Author: vlo
 *
 * Created on 13. Juli 2013, 17:00
 */

#ifndef CMDLINE_H
#define	CMDLINE_H

#include "player.h"
#include "application.h"
#include "output.h"
#include "protracker.h"

void cmdline_set_default_config_player(player_t * player);
void cmdline_set_default_config_output(output_opts_t * output_opts);
int cmdline_parse(int argc, char ** argv, modplay_application_t * app);
void cmdline_usage (char * prog);

#endif	/* CMDLINE_H */

