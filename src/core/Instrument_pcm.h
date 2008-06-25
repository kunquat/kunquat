

/*
 * Copyright 2008 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_INSTRUMENT_PCM_H
#define K_INSTRUMENT_PCM_H


#include <Instrument.h>


int Instrument_pcm_init(Instrument* ins);


void Instrument_pcm_uninit(Instrument* ins);


void Instrument_pcm_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq);


#endif // K_INSTRUMENT_PCM_H


