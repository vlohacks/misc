#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "application.h"
#include "player.h"
#include "module.h"
#include "loader.h"
#include "ui_terminal.h"
#include "ui_ncurses.h"
#include "output_alsa.h"
#include "output_portaudio.h"

#include "output.h"
#include "cmdline.h"

int main (int argc, char ** argv)
{

    modplay_application_t app;  
    int i;
    float l, r;
    
    app.player = player_init(44100.0f, player_resampling_linear);
    app.output_opts = malloc(sizeof(output_opts_t));
    app.loop_playlist = 0;
    app.running = 1;
    
    if (cmdline_parse(argc, argv, &app)) {
        return 1;
    }

    ui_ncurses_init();
    
    output_portaudio_init(app.output_opts);
    //output_alsa_init(0, 0);
    
    
    /*
    ui_terminal_init();
    player_register_order_callback(app.player, ui_terminal_print_order_info);
    player_register_row_callback(app.player, ui_terminal_print_row_info);
    */
    
    
    
    
    player_register_order_callback(app.player, ui_ncurses_order_handler);
    player_register_row_callback(app.player, ui_ncurses_row_handler);
    player_register_tick_callback(app.player, ui_ncurses_tick_handler);
    player_register_channel_sample_callback(app.player, ui_ncurses_channel_sample_handler, 0b11111111111);
    
    
    while (app.running) {
        
        for (i = 0; i < app.playlist_count; i++) {
            
            module_t * mod = loader_loadfile_by_extension(app.playlist[i]);
            
            player_set_module(app.player, mod);
            
            ui_ncurses_new_song_handler(mod);
            //ui_terminal_print_moduleinfo(mod);
            
            output_portaudio_start(app.player);

            app.player->playing = 1;
            while (app.player->playing) {
                // TODO here: UI input stuff etc.
                output_portaudio_wait();
                //player_read(app.player, &l, &r);
                //output_alsa_write(l, r);
            }

            output_portaudio_stop();
            module_free(mod);
        }
        
        if (!app.loop_playlist)
            app.running = 0;
    }
    
    
    
    player_free(app.player);
    free(app.output_opts);
    output_portaudio_cleanup();
    
    ui_ncurses_cleanup();
            
    return 0;
    
}

