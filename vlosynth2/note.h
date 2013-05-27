#ifndef __NOTE_H
#define __NOTE_H

#include "types.h"
#include "math.h"

typedef enum {
	A	= 0,
	Cb	= 1,
	C 	= 2,
	Db	= 3,
	D 	= 4,
	Eb	= 5,
	E	= 6,
	F	= 7,
	Gb	= 8,
	G	= 9,
	Ab	= 10
} note_t;

scalar_t note2freq(note_t note, int octave);

#endif
