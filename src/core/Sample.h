

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#ifndef K_SAMPLE_H
#define K_SAMPLE_H


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <frame_t.h>
#include <Voice_state.h>
#include <Generator.h>


typedef enum
{
    /// Uninitialised.
    SAMPLE_FORMAT_NONE,
    /// The native format.
    // SAMPLE_FORMAT_KS,
    /// WavPack.
    SAMPLE_FORMAT_WAVPACK,
    /// Vorbis.
    // SAMPLE_FORMAT_VORBIS,
    /// Sentinel -- not a valid format.
    SAMPLE_FORMAT_LAST
} Sample_format;


/**
 * Sample contains a digital sound sample.
 */
typedef struct Sample
{
    /// The path of the file (if applicable -- otherwise NULL).
    char* path;
    /// The (compression) format.
    Sample_format format;
    /// Whether the sample (sound) data has changed after loading.
    bool changed;
    /// Whether this sample uses lossy compression.
    bool is_lossy;
    /// The number of channels (1 or 2).
    int channels;
    /// The bit resolution (8, 16, 24 or 32).
    int bits;
    /// Whether this sample is in floating point format.
    bool is_float;
    /// The length of the sample (in amplitude values per channel).
    uint64_t len;
    /// The playback frequency used to represent 440 Hz tone.
    double mid_freq;
    /// The sample data.
    void* data[2];
} Sample;


/**
 * Creates a new Sample.
 *
 * \return   The new Sample if successful, or \c NULL if memory allocation
 *           failed.
 */
Sample* new_Sample(void);


/**
 * Loads contents from a file into a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param in       The input file -- must not be \c NULL.
 * \param format   The input file format -- must be valid.
 */
bool Sample_load(Sample* sample, FILE* in, Sample_format format);


/**
 * Loads a given path into a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param path     The input file path -- must not be \c NULL.
 * \param format   The input file format -- must be valid.
 */
bool Sample_load_path(Sample* sample, char* path, Sample_format format);


/**
 * Gets the path of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The path.
 */
char* Sample_get_path(Sample* sample);


/**
 * Sets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param freq     The middle frequency -- must be > \c 0.
 */
void Sample_set_freq(Sample* sample, double freq);


/**
 * Gets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The middle frequency.
 */
double Sample_get_freq(Sample* sample);


/**
 * Mixes a Sample.
 *
 * \param sample    The Sample -- must not be \c NULL.
 * \param gen       The Generator containing the Sample -- must not be
 *                  \c NULL.
 * \param state     The Voice state -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 * \param offset    The buffer offset.
 * \param freq      The mixing frequency -- must be > \c 0.
 */
void Sample_mix(Sample* sample,
        Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq);


/**
 * Destroys a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 */
void del_Sample(Sample* sample);


#endif // K_SAMPLE_H


