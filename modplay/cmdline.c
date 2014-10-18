#include "cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cmdline_set_default_config_player(player_t * player)
{
    player->resampling = player_resampling_linear;
    player->paula_freq_index = PAL;
    player->loop_module = 0;
    player->loop_pattern = -1;
    player->solo_channel = -1;
}

void cmdline_set_default_config_output(output_opts_t * output_opts)
{
    output_opts->buffer_size = 1024;
    output_opts->sample_rate = 44100;
}

int cmdline_parse_output_opts(char * cmdline, modplay_application_t * app)
{
    char * p;
    
    app->output_opts->driver = output_driver_portaudio; // default 
    
    if (!strncmp(cmdline, "raw:", 4)) {
        p = strstr(cmdline, ":");
        if (p != 0) {
            p++;
            // colon present but no text after it...
            if ((*p) == 0) {
                p = 0;
            } else {
                app->output_opts->output_device = p;
            }
        }
        if (p == 0) {
            fprintf(stderr, "You have to specify a file name for raw output!\n");
            return 1;
        }
        app->output_opts->driver = output_driver_raw;
    }
    
    if (!strncmp(cmdline, "benchmark", 9)) {
        app->output_opts->driver = output_driver_benchmark;
    }
    
    return 0;    
}

int cmdline_parse(int argc, char ** argv, modplay_application_t * app) {
    int c;
    
    cmdline_set_default_config_player(app->player);    
    cmdline_set_default_config_output(app->output_opts);
    
    if (argc < 2) {
        cmdline_usage(argv[0]);
        return 1;
    }

    while ((c = getopt(argc, argv, "i:r:f:b:hPlLp:s:o:")) != -1) {
        switch (c) {
            case 'i':           // interpolation
                if (!strcmp(optarg, "linear")) {
                    app->player->resampling = player_resampling_linear;
                } else if (!strcmp(optarg, "none")) {
                    app->player->resampling = player_resampling_none;
                } else { 
                    fprintf(stderr, "illegal interpolation option: %s\n", optarg);
                }
                    
                break;
                
            case 'r':           // sample rate
                app->output_opts->sample_rate = atoi(optarg);
                app->player->sample_rate = (float)app->output_opts->sample_rate;
                break;
                
            case 'f':           // amiga timing
                if (!strcmp(optarg, "ntsc")) {
                    app->player->paula_freq_index = NTSC;
                } else if (!strcmp(optarg, "pal")) {
                    app->player->paula_freq_index = PAL;
                } else { 
                    fprintf(stderr, "illegal amiga frequency option: %s\n", optarg);
                }
                break;
                
            case 'b':           // output buffer size
                app->output_opts->buffer_size = atoi(optarg);
                break;
                
            case 'P':           // Protracker strict mode
                player_set_protracker_strict_mode(app->player, 1);
                break;
                
            case 'L':           // loop single module
                app->player->loop_module = 1;
                break;
                
            case 'l':           // loop playlist
                app->loop_playlist = 1;
                break;
                
            case 'p':
                app->player->loop_pattern = atoi(optarg);
                break;
                
            case 's':
                app->player->solo_channel = atoi(optarg);
                break;
                
            case 'o':
                if (cmdline_parse_output_opts(optarg, app))
                    return 1;
                //app->output_config = optarg;
                break;
                
            case '?':
                switch (optopt) {
                    case 'o':
                    case 'r':
                    case 'i':
                    case 'f':
                    case 'b':
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
        app->playlist = 0;
        app->playlist_count = 0;
        return 1;
    } else {
        app->playlist = &argv[optind];
        app->playlist_count = argc - optind;
    }
            
    return 0;
}

void cmdline_usage (char * prog)
{
    fprintf (stderr, 
            "Usage: %s [options] <modulefile> [<modulefile2> ...]\n\n"
            "Options:\n"
            "   -r <rate>               Use sample rate <rate>, default is 44100\n"
            "   -i <interpolation>      Use interpolation type <interpolation>\n"
            "                           possible values: none, linear. Default is linear\n"
            "   -P                      Enable protracker strict mode:\n"
            "                           no panning, only 3 octaves\n"
            "   -f <amiga frequency>    Set the amiga paula base frequecy\n"
            "                           possible values: pal, ntsc. Default is pal.\n"
            "   -b <buffer size>        Set audio output buffer size. Default is 1024\n"
            "   -l                      Loop the modules list\n"
            "   -L                      Loop single module\n"
            "   -o                      Output configuration, possible values:\n"
#ifndef DOS            
            "                                   portaudio               use portaudio\n"
#endif            
            "                                   raw:<filename.ext>      raw output to <filename.ext>\n"
            "                                   benchmark               show samples/sec performance (not actually making noise)"
            "   -h                      This text\n"
            "   -s <channel>            solo <channel>\n"
            "   -p <pattern>            loop <pattern>\n", prog);
    

}