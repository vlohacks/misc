/* 
 * File:   output_winmm.h
 * Author: vlo
 *
 * Created on 6. Juli 2013, 20:29
 */

#ifndef OUTPUT_WINMM_H
#define	OUTPUT_WINMM_H

int output_winmm_init(int output_argc, char ** output_argv);
int output_winmm_cleanup();
int output_winmm_write(float l, float r);

#endif	/* OUTPUT_WINMM_H */

