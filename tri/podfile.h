/* 
 * File:   podfile.h
 * Author: vlo
 *
 * Created on 27. September 2014, 21:37
 */

#ifndef PODFILE_H
#define	PODFILE_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    char name[32];
    char * palette;
    uint32_t size;
    uint32_t offset;
} podfile_asset_t;

typedef struct {
    char description[80];
    uint32_t num_assets;
    podfile_asset_t ** assets;
    FILE * f;    
} podfile_t;

podfile_t * podfile_open(const char * path);
int podfile_close(podfile_t * podfile);
podfile_asset_t * podfile_get_asset_by_name(const char * name, const podfile_t * pod);
int podfile_get_asset_data(char * ptr, const podfile_t * pod, const podfile_asset_t * asset);
FILE * podfile_get_asset_fileptr(const podfile_t * pod, const podfile_asset_t * asset);
        
#endif	/* PODFILE_H */

