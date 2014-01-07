#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "io_file.h"


io_handle_t io_file_open(char * filename, char * mode)
{
    FILE * f;
    io_handle_t * handle;
    handle = (io_handle_t *)malloc(sizeof(io_handle_t));
    if (handle) {
        f = fopen(filename, mode);
        if (f == 0) {
            free(handle);
            return 0;
        }
        handle->native_handle = f;
        handle->read = io_file_read;
        handle->write = io_file_write;
        handle->tell = io_file_tell;
        handle->seek = io_file_seek;
    }
    return handle;
}

int io_file_close(io_handle_t * handle) 
{
    return fclose((FILE *)handle->native_handle);
}

size_t io_file_read(void * ptr, size_t size, size_t n, io_handle_t * handle)
{
    return fread(ptr, size, n, (FILE *)handle->native_handle);
}

size_t io_file_write(const void * ptr, size_t size, size_t n, io_handle_t * handle)
{
    return fwrite(ptr, size, n, (FILE *)handle->native_handle);
}

size_t io_file_tell(io_handle_t * handle)
{
    return ftell((FILE *)handle->native_handle);
}

int io_file_seek(struct io_handle_t * handle, size_t n, io_seek_direction_t direction)
{
    int whence = SEEK_SET;
    switch (direction) {
        case io_seek_cur: whence = SEEK_CUR; break;
        case io_seek_set: whence = SEEK_SET; break;
        case io_seek_end: whence = SEEK_END; break;
    }
    return fseek((FILE *)handle->native_handle, n, whence);
}