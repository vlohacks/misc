#include "io_mem.h"
#include "io.h"

#include <string.h>
#include <stdlib.h>

io_handle_t * io_mem_open(void * ptr, size_t size)
{
    io_mem_native_handle_t * native_handle;
    io_handle_t * handle = (io_handle_t *)malloc(sizeof(io_handle_t));
    
    if (handle) {
        native_handle = (io_mem_native_handle_t *)malloc(sizeof(io_mem_native_handle_t));
        if (native_handle == 0) {
            free(handle);
            return 0;
        }
        native_handle->pos = 0;
        native_handle->size = size;
        native_handle->ptr = ptr;
        handle->native_handle = native_handle;
        handle->read = io_mem_read;
        handle->seek = io_mem_seek;
        handle->tell = io_mem_tell;
        handle->write = io_mem_write;
    }
    
    return handle;
}

int io_mem_close(io_handle_t * handle) {
    free((io_mem_native_handle_t *)handle->native_handle);
    free(handle);
    return 0;
}

size_t io_mem_read(void * ptr, size_t size, size_t n, io_handle_t * handle)
{
    size_t prod = size * n;
    io_mem_native_handle_t * native_handle = (io_mem_native_handle_t *)handle->native_handle;
    if (prod > (native_handle->size - native_handle->pos))
        prod = (native_handle->size - native_handle->pos);
    memcpy(ptr, (void *)(native_handle->ptr + native_handle->pos), prod);
    native_handle->pos += prod;
    return prod;
}

size_t io_mem_write(const void * ptr, size_t size, size_t n, io_handle_t * handle)
{
    size_t prod = size * n;
    io_mem_native_handle_t * native_handle = (io_mem_native_handle_t *)handle->native_handle;
    if (prod > (native_handle->size - native_handle->pos))
        prod = (native_handle->size - native_handle->pos);
    memcpy((void *)(native_handle->ptr + native_handle->pos), ptr, prod);
    native_handle->pos += prod;
    return prod;
}

size_t io_mem_tell(io_handle_t * handle)
{
    io_mem_native_handle_t * native_handle = (io_mem_native_handle_t *)handle->native_handle;
    return native_handle->pos;
}

int io_mem_seek(struct io_handle_t * handle, size_t n, io_seek_direction_t direction)
{
    io_mem_native_handle_t * native_handle = (io_mem_native_handle_t *)handle->native_handle;
    switch (direction) {
        case io_seek_cur: 
            native_handle->pos += n;
            break;
            
        case io_seek_set:
            native_handle->pos = n;
            break;

        case io_seek_end: 
            native_handle->pos = native_handle->size - n;
            break;
    }
    if (native_handle->pos > native_handle->size)
        native_handle->pos = native_handle->size;
    
    return 0;
}
