#include "loader.h"
#include "io_file.h"

#define LOADER_FORMATS_COUNT 2

static const loader_generic_t loader_formats[] = {
    { loader_mod_check, loader_mod_load, "mod", "Protracker/Fasttracker MOD" },
    { loader_s3m_check, loader_s3m_load, "s3m", "Scream Tracker 3" }
};

module_t * loader_loadfile_by_header(char * filename)
{
    int i;
    io_handle_t * h = io_file_open(filename, "rb");
    module_t * module = 0;
    
    if (h) {
        
        for (i = 0; i < LOADER_FORMATS_COUNT; i++) {
            //printf ("Checking if file is %s ... ", loader_formats[i].description);
            if (loader_formats[i].check(h)) {
                //printf("YES, loading\n");
                module = loader_formats[i].load(h);
                break;
            } else {
                //printf("NO\n");
            }
        } 
        
        io_file_close(h);
    }
    
    return module;
    
}

module_t * loader_loadfile_by_extension(char * filename)
{
    char tmp[4];
    int i;
    io_handle_t * h = io_file_open(filename, "rb");
    char * ext = filename + (strlen(filename) - 3);
    module_t * module = 0;

    if (h) {    
        tmp[3] = 0;
        strncpy (tmp, ext, 3);

        // convert to lower case
        for (i = 0; i < 3; i++) {
            if ((tmp[i] <= 90) && tmp[i] >= 65)
                tmp[i] += 32;
        }

        for (i = 0; i < LOADER_FORMATS_COUNT; i++) {
            if (!strcmp(tmp, loader_formats[i].extension)) {
                module = loader_formats[i].load(h);
                break;
            }
        }
        
        io_file_close(h);
    }

    
    if (!module)
        printf("oh no, could not recognize: >%s< >%s<\n", filename, tmp);
        
    return module;    
    
}
