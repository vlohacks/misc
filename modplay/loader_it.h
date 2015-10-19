/* 
 * File:   loader_it.h
 * Author: vlo
 *
 * Created on 28. August 2015, 19:42
 */

#ifndef LOADER_IT_H
#define	LOADER_IT_H

#include "module.h"
#include "io.h"

int loader_it_check(io_handle_t * h);
module_t * loader_it_load(io_handle_t * h);

#endif	/* LOADER_IT_H */

