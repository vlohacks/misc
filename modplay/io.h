/* 
 * File:   io.h
 * Author: vlo
 *
 * Created on 7. Januar 2014, 17:50
 */

#ifndef IO_H
#define	IO_H

#include <stdio.h>

struct io_handle_t;

typedef enum {
    io_seek_set,
    io_seek_end,
    io_seek_cur
} io_seek_direction_t;

typedef size_t (*io_read_t)(void * ptr, size_t size, size_t n, struct io_handle_t * handle);
typedef size_t (*io_write_t)(const void * ptr, size_t size, size_t n, struct io_handle_t * handle);
typedef size_t (*io_tell_t)(struct io_handle_t * handle);
typedef int (*io_seek_t)(struct io_handle_t * handle, size_t n, io_seek_direction_t direction);
typedef int (*io_feof_t)(struct io_handle_t * handle);

struct io_handle_t {
    void * native_handle;
    io_read_t read;
    io_write_t write;
    io_seek_t seek;
    io_tell_t tell;
    io_feof_t feof;
};

typedef struct io_handle_t io_handle_t;




#endif	/* IO_H */
        