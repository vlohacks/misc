/* 
 * File:   io_mem.h
 * Author: vlo
 *
 * Created on 7. Januar 2014, 19:55
 */

#ifndef IO_MEM_H
#define	IO_MEM_H

#include <stdio.h>
#include "io.h"

typedef struct {
    char * ptr;
    size_t size;
    size_t pos;
} io_mem_native_handle_t;

io_handle_t * io_mem_open(void * ptr, size_t size);
int io_mem_close(io_handle_t * handle);
size_t io_mem_read(void * ptr, size_t size, size_t n, io_handle_t * handle);
size_t io_mem_write(const void * ptr, size_t size, size_t n, io_handle_t * handle);
size_t io_mem_tell(io_handle_t * handle);
int io_mem_seek(struct io_handle_t * handle, size_t n, io_seek_direction_t direction);
int io_mem_feof(struct io_handle_t * handle);

#endif	/* IO_MEM_H */

