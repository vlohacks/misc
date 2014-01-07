/* 
 * File:   loader_s3m.h
 * Author: vlo
 *
 * Created on 19. Dezember 2013, 04:01
 * created while my stomach aches due to a gigantic schnitzel
 * kudos to Schatten Söflingen ;-)
 */

#ifndef LOADER_S3M_H
#define	LOADER_S3M_H

#include "module.h"
#include "io.h"

module_t * loader_s3m_loadfile(char * filename);
module_t * loader_s3m_load(io_handle_t * h);

#endif	/* LOADER_S3M_H */

