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
#include "loader_mod.h"
#include "loader_s3m.h"

module_t * loader_loadfile_by_extension(char * filename);

#endif	/* LOADER_H */

