#include "cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void set_default_config_player(player_t * player)
{
    player->resampling = player_resampling_linear;
    player->paula_freq_index = PAL;
    player->loop_module = 0;
}

void set_default_config_output(output_opts_t * output_opts)
{
    output_opts->buffer_size = 1024;
    output_opts->sample_rate = 44100;
}

int cmdline_parse(int argc, char ** argv, player_t * player, output_opts_t * output_opts, char *** module_files, int * module_files_count) {
    int c;
    
    set_default_config_player(player);    
    set_default_config_output(output_opts);
    
    if (argc < 2) {
        cmdline_usage(argv[0]);
        return 1;
    }

    while ((c = getopt(argc, argv, "i:r:f:b:hP")) != -1) {
        switch (c) {
            case 'i':           // interpolation
                if (!strcmp(optarg, "linear")) {
                    player->resampling = player_resampling_linear;
                } else if (!strcmp(optarg, "none")) {
                    player->resampling = player_resampling_none;
                } else { 
                    fprintf(stderr, "illegal interpolation option: %s\n", optarg);
                }
                    
                break;
                
            case 'r':           // sample rate
                output_opts->sample_rate = atoi(optarg);
                player->sample_rate = (float)output_opts->sample_rate;
                break;
                
            case 'f':
                if (!strcmp(optarg, "ntsc")) {
                    player->paula_freq_index = NTSC;
                } else if (!strcmp(optarg, "pal")) {
                    player->paula_freq_index = PAL;
                } else { 
                    fprintf(stderr, "illegal amiga frequency option: %s\n", optarg);
                }
                break;
                
            case 'b':
                output_opts->buffer_size = atoi(optarg);
                break;
                
            case 'P':
                player_set_protracker_strict_mode(player, 1);
                break;
                
            case '?':
                switch (optopt) {
                    case 'r':
                    case 'i':
                        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                        return 1;
                        break;
                        
                    default:
                        if ((optopt >= 48) && (optopt <= 122))
                            fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                        else
                            fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                        return 1;
                        
                        break;
                }
            case 'h':
            default:
                cmdline_usage(argv[0]);
                return 1;
                break;
        }
    }
    
    if (optind > (argc - 1)) {
        fprintf(stderr, "error: no mod file specified\n");
        *module_files = 0;
        *module_files_count = 0;
        return 1;
    } else {
        *module_files = &argv[optind];
        *module_files_count = argc - optind;
    }
            
    return 0;
}

void cmdline_usage (char * prog)
{
    fprintf (stderr, 
            "Usage: %s [options] <modulefile>\n\n"
            "Options:\n"
            "   -r <rate>               Use sample rate <rate>, default is 44100\n"
            "   -i <interpolation>      Use interpolation type <interpolation>\n"
            "                           possible values: none, linear. Default is linear\n"
            "   -P                      Enable protracker strict mode:\n"
            "                           no panning, only 3 octaves\n"
            "   -f <amiga frequency>    Set the amiga paula base frequecy\n"
            "                           possible values: pal, ntsc. Default is pal.\n"
            "   -b <buffer size>        Set audio output buffer size. Default is 1024\n"
            "   -h                      This text\n", prog);

}