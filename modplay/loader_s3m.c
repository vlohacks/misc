#include "loader_s3m.h"
#include <stdio.h>
#include <string.h>

module_t * loader_s3m_loadfile(char * filename)
{
    char signature[4];
    int r;
    
    module_t * module = (module_t *)malloc(sizeof(module_t));
    FILE * f = fopen(filename, "rb");
    
    if (!module || !f) {
        if (f)
            fclose(f);
        if (module)
            free(module);
        return 0;
    }    
    
    module->module_type = module_type_s3m;

    /* chech if we really deal with a S3M file */
    fseek(f, 0x2c, SEEK_SET);
    r = fread(signature, 1, 4, f);
    if (memcmp(signature, "SCRM", 4)) {
        fclose(f);
        free(module);
        return 0;
    }
    
    /* read the song name */
    fseek(f, 0, SEEK_SET);
    r = fread(module->song_title, 1, 28, f);
    
    /* TODO: we currently ignore GLOBAL_VOLUME .. maybe it will turn out that
     it is a good idea to deal with it... */
    
    /* read num_orders, num_samples, num_patterns */
    
    return module;
}

