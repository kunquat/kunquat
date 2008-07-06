

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


#include <math.h>

#include <Sample.h>
#include <Instrument.h>


#define PCM_SAMPLES_MAX (512)


int Instrument_pcm_init(Instrument* ins);


void Instrument_pcm_uninit(Instrument* ins);


void Instrument_pcm_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq);


/**
 * Loads a Sample into the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL and must be a PCM
 *                instrument.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX. Any previously loaded Sample in the
 *                index will be removed.
 * \param path    The input path. If this is \c NULL, the Sample stored in
 *                \a index will be removed.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_pcm_set_sample(Instrument* ins,
		uint16_t index,
		char* path);


/**
 * Gets a Sample from the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL and must be a PCM
 *                instrument.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 *
 * \return   The Sample if one exists, otherwise \c NULL.
 */
Sample* Instrument_pcm_get_sample(Instrument* ins, uint16_t index);


/**
 * Gets the path of a Sample in the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL and must be a PCM
 *                instrument.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 *
 * \return   The path of the sample if one exists, otherwise \c NULL.
 */
char* Instrument_pcm_get_path(Instrument* ins, uint16_t index);


/**
 * Sets the middle frequency of a Sample.
 *
 * \param ins     The Instrument -- must not be \c NULL and must be a PCM
 *                instrument.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 * \param freq    The middle frequency -- must be > \c 0.
 */
void Instrument_pcm_set_sample_freq(Instrument* ins,
		uint16_t index,
		double freq);


/**
 * Gets the middle frequency of a Sample.
 *
 * \param ins     The Instrument -- must not be \c NULL and must be a PCM
 *                instrument.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 *
 * \return   The middle frequency of the sample if one exists, otherwise \c 0.
 */
double Instrument_pcm_get_sample_freq(Instrument* ins, uint16_t index);


#endif // K_INSTRUMENT_PCM_H


