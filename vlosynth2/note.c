#include "note.h"
#include "types.h"

scalar_t note2freq(note_t note, int octave) 
{
	const scalar_t sqr2 = 1.0594630943593;
	scalar_t freq = 440 * pow(sqr2, note);

	if (octave < 4)
		freq /=  (scalar_t)(1 << (4 - octave));
	else 
		freq *= (scalar_t)(1 << (octave - 4));

	return freq;
}

