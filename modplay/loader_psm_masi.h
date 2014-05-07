/* 
 * File:   newfile.h
 * Author: vlo
 *
 * Created on 01. May 2014, 10:26
 */

#ifndef LOADER_STM_H
#define	LOADER_STM_H

#include "module.h"
#include "io.h"


const char * const LOADER_PSM_MASI_HEADER = { 'P', 'S', 'M', 0x20 };

int loader_psm_masi_check(io_handle_t * h);
module_t * loader_psm_masi_load(io_handle_t * h);

#endif	/* LOADER_STM_H */

