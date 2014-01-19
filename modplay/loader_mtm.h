/* 
 * File:   loader_mtm.h
 * Author: vlo
 *
 * Created on 18. Januar 2014, 22:07
 */

#ifndef LOADER_MTM_H
#define	LOADER_MTM_H

#include "io.h"
#include "module.h"

int loader_mtm_check(io_handle_t * h);
module_t * loader_mtm_load(io_handle_t * h);


#endif	/* LOADER_MTM_H */

