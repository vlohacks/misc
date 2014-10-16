#include "podfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

podfile_t *  podfile_open(const char * path) 
{
    int i;
    int tmp;

    podfile_t * pod = (podfile_t *)malloc(sizeof(podfile_t));
    podfile_asset_t * asset;
    
    pod->f = fopen(path, "rb");
    
    if (pod->f == 0) {
        free(pod);
        return 0;
    }
    
    fread(&pod->num_assets, sizeof(uint32_t), 1, pod->f);
    fread(pod->description, sizeof(char), 80, pod->f);
    
    pod->assets = (podfile_asset_t **)malloc(sizeof(podfile_asset_t *) * pod->num_assets);
    for (i = 0; i < pod->num_assets; i++) {
        asset = (podfile_asset_t *)malloc(sizeof(podfile_asset_t));
        fread(asset->name, sizeof(char), 32, pod->f);
        fread(&(asset->size), sizeof(uint32_t), 1, pod->f);
        fread(&(asset->offset), sizeof(uint32_t), 1, pod->f);
        tmp = strlen(asset->name);
        if (tmp < 31) {
            if (asset->name[tmp+1] != 0) {
                asset->palette = &(asset->name[tmp+1]);
            } else {
                asset->palette = 0;
            }
        }
        pod->assets[i] = asset;
    }
    
    return pod;
}

int podfile_close(podfile_t * pod) {
    int i;
    fclose(pod->f);
    for (i = 0; i < pod->num_assets; i++)
        free(pod->assets[i]);
    
    free(pod);
    return 0;
}

podfile_asset_t * podfile_get_asset_by_name(const char * name, const podfile_t * pod) {
    int i;
    for (i = 0; i < pod->num_assets; i++) {
        if (!strcmp(pod->assets[i]->name, name)) {
            return pod->assets[i];
        }
    }
    
    return 0;
}

FILE * podfile_get_asset_fileptr(const podfile_t * pod, const podfile_asset_t * asset) {
    fseek(pod->f, asset->offset, SEEK_SET);
    return pod->f;
}

int podfile_get_asset_data(char * ptr, const podfile_t * pod, const podfile_asset_t * asset) {
    fseek(pod->f, asset->offset, SEEK_SET);
    int i;
    for (i = 0; i < asset->size; i++) 
        fread(ptr++, 1, 1, pod->f);
    
    return 0;
}