/* 
 * File:   loader.h
 * Author: vlo
 *
 * Created on 20. Dezember 2013, 19:01
 */

#ifndef LOADER_H
#define	LOADER_H

#include <string.h>
#include "module.h"
#include "io.h"
#include "loader_mod.h"
#include "loader_s3m.h"
#include "loader_mtm.h"
#include "loader_stm.h"
#include "loader_psm_masi.h"
#include "loader_it.h"

module_t * loader_loadmem_by_header(void * ptr, size_t size);
module_t * loader_loadfile_by_header(char * filename);
module_t * loader_loadfile_by_extension(char * filename);

typedef struct {
    int (*check)(io_handle_t * h);
    module_t * (*load)(io_handle_t * h);
    char * extension;
    char * description;
} loader_generic_t;

#endif	/* LOADER_H */

