/* 
 * File:   io_file.h
 * Author: vlo
 *
 * Created on 7. Januar 2014, 18:00
 */

#ifndef IO_FILE_H
#define	IO_FILE_H

#include "io.h"

io_handle_t * io_file_open(char * filename, char * mode);
int io_file_close(io_handle_t * handle);
size_t io_file_read(void * ptr, size_t size, size_t n, io_handle_t * handle);
size_t io_file_write(const void * ptr, size_t size, size_t n, io_handle_t * handle);
size_t io_file_tell(io_handle_t * handle);
int io_file_seek(struct io_handle_t * handle, size_t n, io_seek_direction_t direction);
        

#endif	/* IO_FILE_H */

