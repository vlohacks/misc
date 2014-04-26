#include "ui_ncurses.h"
#include "ui.h"
#include "player_command.h"
#include "string.h"
//#include <signal.h>


#define UI_NCURSES_KEYBINDINGS_COUNT 6

static ui_ncurses_keybinding_t ui_ncurses_keybindings[] = {
    { "\x1b", "ESC", "Quit player", player_command_action_quit, 0 },
    { "\x1b\x5b\x31\x3b\x35\x43", "STRG + ->", "Skip to next order", player_command_action_next_order, 0 },
    { "\x1b\x5b\x31\x3b\x35\x44", "STRG + ->", "Skip to previous order", player_command_action_prev_order, 0 },
    { "\x1b\x5b\x31\x3b\x33\x43", "ALT + ->", "Skip to next song", player_command_action_next_song, 0 },
    { "\x1b\x5b\x31\x3b\x33\x44", "ALT + ->", "Skip to previous song", player_command_action_prev_song, 0 },
    { "\x1b\x5b\x32\x34\x7e", "F12", "Show log", 0, ui_ncurses_show_help }
};


volatile int term_resized;
ui_ncurses_layout_t ui_ncurses_layout;

void ui_ncurses_init()
{
    ui_ncurses_layout.song_view = 0;
    ui_ncurses_layout.channel_view = 0;
    ui_ncurses_layout.pattern_view = 0;
    ui_ncurses_layout.ncurses_inited = 0;
    ui_ncurses_layout.num_channels = 4;
    ui_ncurses_layout_init();
    term_resized = 0;
    
    //signal(SIGWINCH, ui_ncurses_term_resized);
}

void ui_ncurses_cleanup()
{
    endwin();
}

void ui_ncurses_term_resized(int i) 
{
    term_resized ++;
}

void ui_ncurses_new_song_handler(module_t * mod)
{
    ui_ncurses_layout.num_channels = mod->num_channels;
    ui_ncurses_layout_init();
}

void ui_ncurses_layout_init()
{
    int h, w;
    ui_ncurses_layout.init = 1;
    
    if (ui_ncurses_layout.channel_view)
        delwin(ui_ncurses_layout.channel_view);
    
    if (ui_ncurses_layout.pattern_view)
        delwin(ui_ncurses_layout.pattern_view);

    if (ui_ncurses_layout.song_view)
        delwin(ui_ncurses_layout.song_view);
    
    if (ui_ncurses_layout.aux)
        delwin(ui_ncurses_layout.aux);
    
    if (ui_ncurses_layout.ncurses_inited)
        endwin();
    
    initscr();
    //curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
    
    ui_ncurses_layout.ncurses_inited = 1;
    ui_ncurses_layout.use_colors = has_colors();
    
    if (ui_ncurses_layout.use_colors) {
        start_color();
        init_pair(UI_NCURSES_COLORPAIR_WINDOW, COLOR_WHITE, COLOR_BLUE);
    }
    
    getmaxyx(stdscr, h, w);
    
    ui_ncurses_layout.current_h = h;
    ui_ncurses_layout.current_w = w;
                
    ui_ncurses_layout.song_view = newwin(1, w, 0, 0);
    ui_ncurses_layout.channel_view = newwin(ui_ncurses_layout.num_channels + 2, w, 1, 0);
    ui_ncurses_layout.pattern_view = newwin((h - (ui_ncurses_layout.num_channels + 3)), w, ui_ncurses_layout.num_channels + 3, 0);
    ui_ncurses_layout.aux = 0;
    
    if (ui_ncurses_layout.use_colors) {
       wattron(ui_ncurses_layout.channel_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
       wattron(ui_ncurses_layout.pattern_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
    }


    

    
    
    
    
    
    if (ui_ncurses_layout.use_colors) {
        wattroff(ui_ncurses_layout.channel_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
        wattroff(ui_ncurses_layout.pattern_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
    }
        

    ui_ncurses_layout.init = 0;

}

void ui_ncurses_order_handler(player_t * player, int current_order, int current_pattern)
{
    mvwprintw(ui_ncurses_layout.song_view, 0, 0, "VloSoft MOD Player | song: %s | ord: %03i/%03i | pat: %03i/%03i", player->module->song_title, current_order, player->module->num_orders, current_pattern, player->module->num_patterns);
    wrefresh(ui_ncurses_layout.song_view);
}

#define SCOPE_SIZE 15

void ui_ncurses_channel_sample_handler(float l, float r, float peak_l, float peak_r, int channel)
{
    char tmp[(SCOPE_SIZE * 2) + 2];
    int i;
    
    for (i = 0; i < SCOPE_SIZE; i++) {
        if ((int)(peak_l*SCOPE_SIZE) >= (SCOPE_SIZE - i)) 
            tmp[i] = '=';
        else 
            tmp[i] = '-';
    }
    
    
    for (i = SCOPE_SIZE; i < SCOPE_SIZE * 2 + 1; i++) {
        if ((int)(peak_r * SCOPE_SIZE) >= (i - SCOPE_SIZE))
            tmp[i] = '=';
        else
            tmp[i] = '-';
    }
    
    
    
    tmp[SCOPE_SIZE] = '|';
    tmp[SCOPE_SIZE * 2 + 1] = 0;
    
    mvwprintw(ui_ncurses_layout.channel_view, channel+1, 80, tmp);
    if (ui_ncurses_layout.aux)
        overwrite(ui_ncurses_layout.aux, ui_ncurses_layout.channel_view);

    wrefresh(ui_ncurses_layout.channel_view);
    
}

void ui_ncurses_tick_handler(player_t * player, int current_order, int current_pattern, int current_row, int current_tick, player_channel_t * channels)
{
    char tmp[100];
    char tmp2[40];
    char tmp3[30];
    char note[4];
    
    int i, j, k;

    
    
    for (i = 0; i < ui_ncurses_layout.num_channels; i++) {
        module_pattern_data_t * data = &(player->module->patterns[current_pattern].rows[current_row].data[i]);
        
        if (data->period_index >= 0)
            wattron(ui_ncurses_layout.channel_view, A_BOLD);
        
        sprintf(note, "...");
        if (channels[i].period_index < 254)
            ui_periodindex2note(channels[i].period_index, note);

        /*
        tmp3[0] = 0;
        k = 0;
        if (channels[i].sample_num > 0) {
            for (j = 0; j < strlen(player->module->samples[channels[i].sample_num - 1].header.name); j++) {
                if (isprint(player->module->samples[channels[i].sample_num - 1].header.name[j])) {
                    tmp3[k++] = player->module->samples[channels[i].sample_num - 1].header.name[j];
                }
            }
            tmp3[k] = 0;
        }
        */
        
        
        
        tmp2[0] = 0;
        if (data->effect_num)
            ui_effect_to_humanreadable(tmp2, data->effect_num, player->channels[i].effect_last_value, player->module->module_type);
        
        sprintf(tmp, "%-30s | %3s | %2i | %02x | %-27s | ", channels[i].sample_num ? player->module->samples[channels[i].sample_num - 1].header.name : "", note, channels[i].volume, channels[i].panning, tmp2);
        mvwprintw(ui_ncurses_layout.channel_view, i+1, 1, tmp);
        wattroff(ui_ncurses_layout.channel_view, A_BOLD);
    }

    if (ui_ncurses_layout.use_colors)
        wattron(ui_ncurses_layout.channel_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
    
    box(ui_ncurses_layout.channel_view, 0, 0);
    
    if (ui_ncurses_layout.use_colors)
        wattroff(ui_ncurses_layout.channel_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));

    if (ui_ncurses_layout.aux)
        overwrite(ui_ncurses_layout.aux, ui_ncurses_layout.channel_view);
    
    wrefresh(ui_ncurses_layout.channel_view);    
}

void ui_ncurses_row_handler(player_t * player, int current_order, int current_pattern, int current_row) 
{
    if (term_resized) {
        ui_ncurses_layout_init();
        term_resized --;
    }
        
    
    int current_pos_location = ui_ncurses_layout.pattern_view->_maxy / 2;
    int i, j, k;
    char note[4];
    char tmp[20];
    char tmp2[400];
    char effect[2];
    char volume[3];
    
    j = ui_ncurses_layout.pattern_view->_begy + 1;
    for (j = 0; j < ui_ncurses_layout.pattern_view->_maxy - 1; j++) {
        
        i = j + (current_row - current_pos_location);
        
        if ((i >= 0) && (i < player->module->patterns[current_pattern].num_rows)) {
            sprintf(tmp2, "%02d|", i);
            for (k = 0; k < player->module->num_channels; k++) {
                module_pattern_data_t * data = &(player->module->patterns[current_pattern].rows[i].data[k]);
                ui_periodindex2note(data->period_index, note);
                
                if (data->volume >= 0)
                    sprintf(volume, "%02i", data->volume);
                else
                    strcpy(volume, "..");

                ui_map_effect_num(effect, player->module->module_type, data->effect_num);
            
                sprintf(tmp, "%s%02i%s%s%02X|", note, data->sample_num, volume, effect, data->effect_value);
                strcat (tmp2, tmp);
            }
        } else {
            for(k = 0; k < ui_ncurses_layout.pattern_view->_maxx - 2; k++)
                tmp2[k] = ' ';
            tmp2[k] = 0;
        }
        if (j == current_pos_location)
            wattron(ui_ncurses_layout.pattern_view, A_REVERSE);
        
        if ((i % 4) == 0)
            wattron(ui_ncurses_layout.pattern_view, A_BOLD);
        
        mvwprintw(ui_ncurses_layout.pattern_view, j+1, 1, tmp2);
        wattroff(ui_ncurses_layout.pattern_view, A_REVERSE);
        wattroff(ui_ncurses_layout.pattern_view, A_BOLD);
    }
    
    if (ui_ncurses_layout.use_colors)
        wattron(ui_ncurses_layout.pattern_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
     
    box(ui_ncurses_layout.pattern_view, 0, 0);

    if (ui_ncurses_layout.use_colors)
        wattroff(ui_ncurses_layout.pattern_view, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));

    if (ui_ncurses_layout.aux)
        overwrite(ui_ncurses_layout.aux, ui_ncurses_layout.pattern_view);
 
    
    wrefresh(ui_ncurses_layout.pattern_view);
    
}

player_command_action_t ui_ncurses_handle_input() 
{
    int c;
    int i;
        
    char buf[12];
    
    i = 0;
    while((c = getch()) != ERR) {
        cbreak();
        buf[i++] = (char)c;
        if (i >= 10)
            break;
    }
   
    buf[i++] = 0;


    if (i > 1) {
        FILE * xx = fopen("xxx.txt", "a");
        fprintf(xx, "\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        fclose (xx);
    }
          
    
    for (i = 0; i < UI_NCURSES_KEYBINDINGS_COUNT; i++) {
        if (!strcmp(ui_ncurses_keybindings[i].keyseq, buf)) {
            if (ui_ncurses_keybindings[i].hook) {
                (ui_ncurses_keybindings[i].hook)();
            } else {
                return ui_ncurses_keybindings[i].action;
            }
        }
        
    }
    
    return player_command_action_none;
}



void ui_ncurses_show_log() {
    
}

void ui_ncurses_destroy_aux() {
    if (ui_ncurses_layout.aux) {
        delwin(ui_ncurses_layout.aux);
        ui_ncurses_layout.aux = 0;
    }
    
    touchwin(ui_ncurses_layout.channel_view);
    touchwin(ui_ncurses_layout.pattern_view);
    
    ui_ncurses_keybindings[0].hook = 0;
}


void ui_ncurses_show_help() {
    int i;


    if (ui_ncurses_layout.aux) 
        return;
       
    
    ui_ncurses_layout.aux = newwin(ui_ncurses_layout.current_h - 8, ui_ncurses_layout.current_w - 8, 4, 4);       
    
    if (ui_ncurses_layout.use_colors)
        wattron(ui_ncurses_layout.aux, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));
    
    box(ui_ncurses_layout.aux, 0, 0);
    
    if (ui_ncurses_layout.use_colors)
        wattroff(ui_ncurses_layout.aux, COLOR_PAIR(UI_NCURSES_COLORPAIR_WINDOW));    
    
    for (i = 0; i < UI_NCURSES_KEYBINDINGS_COUNT; i++) {
        mvwprintw(ui_ncurses_layout.aux, i+1, 1, "%-12s %s", ui_ncurses_keybindings[i].keyname, ui_ncurses_keybindings[i].description);
    }
    
    ui_ncurses_keybindings[0].hook = ui_ncurses_destroy_aux;
    
    
}