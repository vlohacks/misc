#include "ui_terminal.h"
#include "module.h"
#include "ui.h"
#include "application.h"
#include <stdio.h>
#include <string.h>

FILE * ui_terminal_fd;


char color_none[] = "";
char color_reset[] = "\x1b[0m";
//char color_bggrey[] = "\x1b[47m\x1b[37;1m";
char color_bggrey[] = "\x1b[37;1m";
char color_lgreen[] = "\x1b[32;1m";
char color_lblue[] = "\x1b[34;1m";
char color_lred[] = "\x1b[31;1m";
char color_lyellow[] = "\x1b[33;1m";
char color_lmagenta[] = "\x1b[35;1m";
char color_lwhite[] = "\x1b[37;1m";
char color_hilight[] = "\x1b[0m\x1b[1m";
char color_dark[] = "\x1b[0m\x1b[2m";

char * color_fx_map[16];

char * color_4throw = color_none;
char * color_otherrow = color_none;
char * color_std = color_none;


void ui_terminal_init() 
{
    int i;
#ifdef WIN32
    int have_ansi = 0;
#else
    int have_ansi = 1;
#endif
    
    for (i=0; i<16; i++)
        color_fx_map[i] = color_none;
    
    ui_terminal_fd = stdout;
    
    if (have_ansi) {
        color_4throw = color_hilight;
        color_otherrow = color_reset;
        color_std = color_reset;

        color_fx_map[0] = color_lgreen;
        color_fx_map[1] = color_lgreen;
        color_fx_map[2] = color_lgreen;
        color_fx_map[3] = color_lgreen;
        color_fx_map[4] = color_lgreen;
        color_fx_map[5] = color_lblue;
        color_fx_map[6] = color_lblue;
        color_fx_map[7] = color_lblue;
        color_fx_map[8] = color_lmagenta;
        color_fx_map[9] = color_lred;
        color_fx_map[10] = color_lblue;
        color_fx_map[11] = color_lyellow;
        color_fx_map[12] = color_lblue;
        color_fx_map[14] = color_none;
        color_fx_map[13] = color_lyellow;
        color_fx_map[15] = color_lyellow;
    }
    
}

void ui_terminal_print_moduleinfo(module_t * module)
{
    int i;
    
    fprintf(ui_terminal_fd, "\n");    
    fprintf(ui_terminal_fd, "song title   : %s\n", module->song_title);
    //fprintf(ui_terminal_fd, "module info  : %s (signature: %s)\n", module->module_info.description, module->module_info.signature);
    // TODO: output module info dependent on type
    fprintf(ui_terminal_fd, "channels     : %i\n", (int)module->num_channels);
    fprintf(ui_terminal_fd, "patterns     : %i\n", (int)module->num_patterns);
    fprintf(ui_terminal_fd, "orders       : %i\n", (int)module->num_orders);
    
    fprintf(ui_terminal_fd, "\n--== Samples ==--\n");
    for (i = 0; i < module->num_samples; i++) {
        module_sample_header_t * sh = &(module->samples[i].header);
        fprintf(ui_terminal_fd, "%22s | v=%2i l=%6i ls=%6i ll=%6i ft=%1i\n", sh->name, sh->volume, sh->length, sh->loop_start, sh->loop_length, (sh->finetune >=8 ? -(16-sh->finetune) : sh->finetune) );
    }
}

void ui_terminal_print_order_info(module_t * module, int current_order, int current_pattern) 
{
    fprintf(ui_terminal_fd, "\n--== order: %i/%i, pattern: %i/%i ==--\n", current_order, module->num_orders, current_pattern, module->num_patterns);
}

void ui_terminal_print_row_info(module_t * module, int current_order, int current_pattern, int current_row)
{
    int i;
    char note[4];
    char volume[3];
    
    char * the_std_color = current_row % 4 ? color_otherrow : color_4throw;
    
    module_pattern_row_t * row = &(module->patterns[current_pattern].rows[current_row]);
    
    fprintf(ui_terminal_fd, "%s%02d |", the_std_color, current_row );
    for (i = 0; i < module->num_channels; i++) {
        ui_periodindex2note(row->data[i].period_index, note);
        
        if (row->data[i].volume >= 0)
            sprintf(volume, "%02i", row->data[i].volume);
        else
            strcpy(volume, "..");
        
        fprintf(ui_terminal_fd, " %s %02d %s %s%01x%02x%s |", 
                note, row->data[i].sample_num, volume, (row->data[i].effect_num || row->data[i].effect_value) ? color_fx_map[row->data[i].effect_num] : the_std_color, row->data[i].effect_num, row->data[i].effect_value, the_std_color);
         
    }
    fprintf(ui_terminal_fd, "%s\n", color_std);
    fflush(ui_terminal_fd);
    
}

