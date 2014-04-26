/* 
 * File:   newfile.h
 * Author: vlo
 *
 * Created on 26. April 2014, 17:59
 */

#ifndef LOADER_STM_H
#define	LOADER_STM_H

#include "module.h"
#include "io.h"

int loader_stm_check(io_handle_t * h);
module_t * loader_stm_load(io_handle_t * h);

#endif	/* LOADER_STM_H */

