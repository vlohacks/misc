/* 
 * File:   newfile.h
 * Author: vlo
 * 
 * Loader for EPIC Megagames .PSM format (Epic Pinball, Jazz Jackrabbit)
 * Reference:
 * http://www.shikadi.net/moddingwiki/ProTracker_Studio_Module#PSM_New_Format
 *
 * Created on 01. May 2014, 10:26
 */

#ifndef LOADER_PSM_H
#define	LOADER_PSM_H

#include "module.h"
#include "io.h"


static const char LOADER_PSM_MASI_HEADER[] = { 'P', 'S', 'M', 0x20 };



typedef struct {
    /* loader phase. 0 means analyze 1 means load
     * the MASI PSM format is organized as IFF file with chunks which
     * MIGHT be scattered around the file and mixed up, so we first need
     * to check how many of each patterns, samples etc are around to allocate 
     * the correct amount of memory for each...
     */
    uint8_t phase;
    
    /* our module we load the data in..
     */
    module_t * module;
} loader_psm_masi_state_t;


typedef struct {
    uint32_t chunk;
    int (*handler)(io_handle_t * h, loader_psm_masi_state_t * state);
} loader_psm_masi_chunk_t;


/*
const loader_psm_masi_chunk_t loader_psm_masi_chunks[] = {
    { .chunk = 0x454c4946, .handler = loader_psm_masi_FILE },       // FILE
    { .chunk = 0x54464453, .handler = loader_psm_masi_SDFT },       // SDFT
    { .chunk = 0x444f4250, .handler = loader_psm_masi_PBOD }       // PBOD
    
};
*/

int loader_psm_masi_check(io_handle_t * h);
module_t * loader_psm_masi_load(io_handle_t * h);

#endif	/* LOADER_PSM_H */

