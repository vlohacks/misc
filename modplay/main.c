#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "application.h"
#include "player.h"
#include "player_command.h"
#include "module.h"
#include "loader.h"
#include "ui_terminal.h"
#include "ui_ncurses.h"

#include "output.h"
#include "cmdline.h"
#include "platform.h"




int main (int argc, char ** argv)
{
    module_t * mod;
    modplay_application_t app;  
    int i;
    float l, r;
    player_command_action_t action;
    
    app.player = player_init(44100.0f, player_resampling_linear);
    app.output_opts = malloc(sizeof(output_opts_t));
    app.loop_playlist = 0;
    app.running = 1;
    //app.ui_flavour = ui_flavour_terminal;
    app.ui_flavour = ui_flavour_curses;
    //app.ui_flavour = ui_flavour_quiet;
    
    if (cmdline_parse(argc, argv, &app)) {
        return 1;
    }
    
    output_init(app.output_opts);
    
    player_register_callback_user_ptr(app.player, &(app.ui_dirty));

    player_register_order_callback(app.player, ui_generic_order_handler);
    player_register_row_callback(app.player, ui_generic_row_handler);
    player_register_tick_callback(app.player, ui_generic_tick_handler);
    
    switch(app.ui_flavour) {
        
        case ui_flavour_curses:
            ui_ncurses_init();    
            break;
            
        case ui_flavour_terminal:
            ui_terminal_init();
            break;
            
        default: 
            break;
    
    }
    
    
    while (app.running) {

        i = 0;
        for (;;) {
            
            if ((mod = loader_loadfile_by_extension(app.playlist[i])) == 0)
                mod = loader_loadfile_by_header(app.playlist[i]);
            
            player_set_module(app.player, mod);
            
            switch (app.ui_flavour) {
                case ui_flavour_curses:
                    ui_ncurses_new_song_handler(mod);
                    break;
                    
                case ui_flavour_terminal:
                    ui_terminal_print_moduleinfo(mod);
                    break;
                    
            }

            output_start(app.player);
            
            while (app.player->playing) {
                
                switch (app.ui_flavour) {
                    case ui_flavour_curses:
                        action = ui_ncurses_handle_input();
                        break;
                }
                
                //fprintf(stderr, "%i\n", action);
                switch(action) {
                    case (player_command_action_next_song):
                        if (i < (app.playlist_count - 1)) {
                            app.player->playing = 0;
                        }
                        break;
                        
                    case (player_command_action_prev_song):
                        if (i > 0) {
                            i-=2;
                            app.player->playing = 0;
                        }
                        break;
                        
                    case (player_command_action_quit):
                        app.player->playing = 0;
                        app.running = 0;
                        break;
                                    
                    default: 
                        player_command_action_dispatch(app.player, action);
                        break;
                    
                }
                // TODO here: UI input stuff etc.
                switch (app.ui_flavour) {
                    case ui_flavour_curses: ui_ncurses_refresh(app.player, &(app.ui_dirty)); break;
                }
#ifndef PLATFORM_DOS                
                output_portaudio_wait();
#endif
                //player_read(app.player, &l, &r);
                //output_alsa_write(l, r);
            }
            
            output_stop();
            module_free(mod);
            
            i++;
            if (i == app.playlist_count)
                break;
            
        }
        
        if (!app.loop_playlist)
            app.running = 0;
    }

    output_cleanup();    
    player_free(app.player);
    free(app.output_opts);
    
    switch (app.ui_flavour) {
        case ui_flavour_curses:
            ui_ncurses_cleanup();
            break;
            
        default:
            break;
    }
            
    return 0;
    
}

