/* 
 * File:   cmdline.h
 * Author: vlo
 *
 * Created on 13. Juli 2013, 17:00
 */

#ifndef CMDLINE_H
#define	CMDLINE_H

#include "player.h"
#include "output.h"
#include "protracker.h"

int cmdline_parse(int argc, char ** argv, player_t * player, output_opts_t * output_opts, char *** module_files, int * module_files_count);
void cmdline_usage (char * prog);

#endif	/* CMDLINE_H */

