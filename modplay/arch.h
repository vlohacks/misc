/* 
 * File:   arch.h
 * Author: vlo
 * 
 * Architecture-specific routines
 *
 * Created on 29. Juni 2013, 13:47
 */

#ifndef ARCH_H
#define	ARCH_H

static inline uint16_t swap_endian_u16(uint16_t i) {
    return ((i >> 8) | (i << 8));
}

static inline uint32_t swap_endian_u32(uint32_t i) {
    return ((i>>24)&0xff) | ((i<<8)&0xff0000) | ((i>>8)&0xff00) | ((i<<24)&0xff000000);
}

#endif	/* ARCH_H */

