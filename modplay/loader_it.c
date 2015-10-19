#include "loader_it.h"
#include <string.h>
#include <stdlib.h>

int loader_it_check(io_handle_t * h)
{
    char signature[4];
    size_t saved_pos;
    
    memset(signature, 0, 4);
    
    saved_pos = h->tell(h);
    h->seek(h, 0x0, io_seek_set);
    h->read(signature, 1, 4, h);
    h->seek(h, saved_pos, io_seek_set);
        
    if (memcmp(signature, "IMPM", 4))
        return 0;

    return 1;
}


module_t * loader_it_load(io_handle_t * h)
{
    module_t * module;
    uint8_t tmp_u8;
    uint8_t has_songmessage = 0;
    uint16_t tmp_u16;
    
    uint32_t offset_ins, offset_smp, offset_pat;
    
    int i;
    
    if (!loader_it_check(h)) 
        return 0;
    
    module = malloc(sizeof(module_t));
    
    // skip 'IMPM'
    h->seek(h, 4, io_seek_cur);
    
    h->read(module->song_title, 26, 1, h);
    
    // skip Pattern Hilight info
    h->seek(h, 2, io_seek_cur);
    
    h->read(&(module->num_orders), sizeof(uint16_t), 1, h);
    h->read(&(module->num_instruments), sizeof(uint16_t), 1, h);
    h->read(&(module->num_samples), sizeof(uint16_t), 1, h);
    h->read(&(module->num_patterns), sizeof(uint16_t), 1, h);
    
    // Version infos
    h->read(&(module->module_info.flags_it.created_with), sizeof(uint16_t), 1, h);
    h->read(&(module->module_info.flags_it.compatible_with), sizeof(uint16_t), 1, h);
    
    // Flags
    h->read(&tmp_u16, sizeof(uint16_t), 1, h);
    if (tmp_u16 & (1<<0))
        module->module_info.flags_it.stereo = 1;
    
    /* (1<<1) - vol 0 optimizations ignored since this is unser bier wie wir
     *  das machen
     */
    
    if (tmp_u16 & (1<<2))
        module->module_info.flags_it.use_instruments = 1;
    
    if (tmp_u16 & (1<<3))
        module->module_info.flags_it.linear_slides = 1;

    if (tmp_u16 & (1<<4))
        module->module_info.flags_it.old_effects = 1;      // IT Mode: Vibrato bei JEDEM tick, irgendwas mit dem Sample offset (need to investigate)
    
    if (tmp_u16 & (1<<5))
        module->module_info.flags_it.g_effect_link_memory = 1;      // link G effect memory with E and F effect , retrigger envelopes

    /* (1<<6 und 1<<7 - Midi Stuff only the tracker needs therefore ignored
     */
    
    // Special
    h->read(&tmp_u16, sizeof(uint16_t), 1, h);
    if (tmp_u16 & (1<<0))
        has_songmessage = 1;
    
    h->read(&(module->initial_master_volume), sizeof(uint8_t), 1, h);
    h->read(&(module->mix_volume), sizeof(uint8_t), 1, h);
    h->read(&(module->initial_speed), sizeof(uint8_t), 1, h);
    h->read(&(module->initial_bpm), sizeof(uint8_t), 1, h);
    h->read(&(module->panning_separation), sizeof(uint8_t), 1, h);
    h->read(&tmp_u8, sizeof(uint8_t), 1, h);                         // MIDI Pitch wheel depth ... ignored
    
    for (i = 0; i < 64; i++)
        h->read(&(module->initial_panning[i]), sizeof(uint8_t), 1, h);    

    for (i = 0; i < 64; i++)
        h->read(&(module->channel_volume[i]), sizeof(uint8_t), 1, h);    
    
    for (i = 0; i < module->num_orders; i++)
        h->read(&(module->orders[i]), sizeof(uint8_t), 1, h);
    
    
    offset_ins = 0x0c + module->num_orders;
    offset_smp = 0x0c + module->num_orders + (module->num_instruments * 4);
    offset_pat = 0x0c + module->num_orders + (module->num_instruments * 4) + (module->num_samples * 4);
    
    return module;
}