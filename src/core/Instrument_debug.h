

#ifndef K_INSTRUMENT_DEBUG_H
#define K_INSTRUMENT_DEBUG_H


#include <Instrument.h>


void Instrument_debug_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq);


#endif // K_INSTRUMENT_DEBUG_H


