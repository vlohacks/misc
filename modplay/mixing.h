/* 
 * File:   mixing.h
 * Author: vlo
 *
 * Created on 1. Mai 2014, 12:21
 * Here macros for mixing types and conversion 
 * Setup for best performance on specific hardware is done here
 */

#ifndef MIXING_H
#define	MIXING_H

#include <stdint.h>

/* this defines what resolution internally is used for sample / mixing ops 
 * Possible options:
 * MIXING_FLOAT: for systems capable of doing fast floating point math
 * MIXING_S16: uses 16 bit signed int for sampling and 32 bit signed int for
 *      multiply/accumulate operations
 * MIXING_S8: uses 8 bit signed int for sampling and 16 bit signed int for
 *      multiply/accumulate operations
 */
#define MIXING_S16

/* this defines how inter-sample calculations are done internally
 * (TODO not yet implemented)
 * SAMPLING_FLOAT: uses float
 * SAMPLING_U24_8: uses 24:8 bit fixed point math (for systems which love 
 *      integer, but hate float
 */
#define SAMPLING_FLOAT





#ifdef MIXING_FLOAT
typedef float sample_t;
typedef float sample_mac_t;
static const sample_t SAMPLE_T_MAX = 1.0f;
static const sample_t SAMPLE_T_MIN = -1.0f;
static const sample_t SAMPLE_T_ZERO = 0.0f;

static inline sample_t sample_from_float(float s) {    return (sample_t)s; }
static inline sample_t sample_from_double(double s) {  return (sample_t)s; }
static inline sample_t sample_from_s16(int16_t s) {    return (sample_t)s / 32768; }
static inline sample_t sample_from_s8(int8_t s) {      return (sample_t)s / 128; }
#endif /* MIXING_FLOAT */


#ifdef MIXING_S16
typedef int16_t sample_t;
typedef int32_t sample_mac_t;
static const sample_t SAMPLE_T_MAX = 32767;
static const sample_t SAMPLE_T_MIN = -32768;
static const sample_t SAMPLE_T_ZERO = 0;

static inline sample_t sample_from_float(float s) {     return (sample_t)(s * 32768); }
static inline sample_t sample_from_double(double s) {   return (sample_t)(s * 32768); }
static inline sample_t sample_from_s16(int16_t s) {     return (sample_t)s; }
static inline sample_t sample_from_s8(int8_t s) {       return (sample_t)s << 8; }
#endif /* MIXING_S16 */

#ifdef MIXING_S8
typedef int8_t sample_t;
typedef int16_t sample_mac_t;
static const sample_t SAMPLE_T_MAX = 127;
static const sample_t SAMPLE_T_MIN = -128;
static const sample_t SAMPLE_T_ZERO = 0;

static inline sample_t sample_from_float(float s) {     return (sample_t)(s * 128); }
static inline sample_t sample_from_double(double s) {   return (sample_t)(s * 128); }
static inline sample_t sample_from_s16(int16_t s) {     return (sample_t)s >> 8; }
static inline sample_t sample_from_s8(int8_t s) {       return (sample_t)s; }
#endif /* MIXING_S8 */

#endif	/* MIXING_H */

