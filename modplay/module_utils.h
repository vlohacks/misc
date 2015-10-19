/* 
 * File:   module_utils.h
 * Author: vlo
 *
 * Created on 19. September 2015, 13:31
 */

#ifndef MODULE_UTILS_H
#define	MODULE_UTILS_H

#include "module.h"
#include <stdio.h>
#include <stdlib.h>

void module_dump_c(module_t * module);
void module_dump(module_t * module, FILE * fd);

#endif	/* MODULE_UTILS_H */

