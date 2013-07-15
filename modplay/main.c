#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "player.h"
#include "module.h"
#include "loader_mod.h"
#include "ui_terminal.h"
//#include "output_alsa.h"
#include "output_portaudio.h"
#include "output.h"
#include "cmdline.h"

int main (int argc, char ** argv)
{

    char ** module_files;
    int num_module_files;
    int i;
    
    output_opts_t output_opts;
    
    player_t * player = player_init(44100.0f, player_resampling_linear);
    
    if (cmdline_parse(argc, argv, player, &output_opts, &module_files, &num_module_files)) {
        return 1;
    }

    output_portaudio_init(&output_opts);
    
    ui_terminal_init();
    
    player_register_order_callback(player, ui_terminal_print_order_info);
    player_register_row_callback(player, ui_terminal_print_row_info);
    
    for (i = 0; i < num_module_files; i++) {
        module_t * mod = loader_mod_loadfile(module_files[i]);
        player_set_module(player, mod);
        
        ui_terminal_print_moduleinfo(mod);
        output_portaudio_start(player);
    
        while (player->playing) {
            // TODO here: UI input stuff etc.
            output_portaudio_wait();
        }
    
        output_portaudio_stop();
        module_free(mod);
    }
        
    player_free(player);
    output_portaudio_cleanup();
            
    return 0;
    
}

