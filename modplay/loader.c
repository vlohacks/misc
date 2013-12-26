#include "loader.h"

module_t * loader_loadfile_by_extension(char * filename)
{
    char tmp[4];
    int i;
    char * ext = filename + (strlen(filename) - 3);
    
    tmp[3] = 0;
    strncpy (tmp, ext, 3);
    
    for (i = 0; i < 3; i++) {
        if ((tmp[i] <= 90) && tmp[i] >= 65)
            tmp[i] += 32;
    }
    
    if (!strcmp(tmp, "mod")) 
        return loader_mod_loadfile(filename);
    
    if (!strcmp(tmp, "s3m")) 
        return loader_s3m_loadfile(filename);
    
    printf("oh no, could not recognize: >%s< >%s<\n", filename, tmp);
        
    return 0;    
    
}
