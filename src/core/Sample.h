

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
    SAMPLE_FORMAT_NONE = 0,
    /// The native format.
    // SAMPLE_FORMAT_KS,
    /// WavPack.
    SAMPLE_FORMAT_WAVPACK,
    /// Vorbis.
    // SAMPLE_FORMAT_VORBIS,
    /// Sentinel -- not a valid format.
    SAMPLE_FORMAT_LAST
} Sample_format;


typedef enum
{
    SAMPLE_LOOP_OFF = 0, ///< No loop.
    SAMPLE_LOOP_UNI,     ///< Unidirectional (forward) loop.
    SAMPLE_LOOP_BI       ///< Bidirectional loop.
} Sample_loop;


/**
 * Sample contains a digital sound sample.
 */
typedef struct Sample
{
    char* path;           ///< The path of the file (if applicable -- otherwise NULL).
    Sample_format format; ///< The (compression) format.
    bool changed;         ///< Whether the sample (sound) data has changed after loading.
    bool is_lossy;        ///< Whether this sample uses lossy compression.
    int channels;         ///< The number of channels (1 or 2).
    int bits;             ///< The bit resolution (8, 16, 24 or 32).
    bool is_float;        ///< Whether this sample is in floating point format.
    Sample_loop loop;     ///< Loop setting.
    uint64_t loop_start;  ///< Loop start.
    uint64_t loop_end;    ///< Loop end (the frame at this index will not be played).
    uint64_t len;         ///< The length of the sample (in amplitude values per channel).
    double mid_freq;      ///< The playback frequency used to represent 440 Hz tone.
    void* data[2];        ///< The sample data.
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
 * Gets the length of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The length in frames/buffer.
 */
uint64_t Sample_get_len(Sample* sample);


/**
 * Sets the loop mode of the Sample.
 *
 * If the loop becomes enabled, the end points may be changed.
 * The loop will not be enabled if the length of the sample is 0.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param loop     The loop mode -- must be \c SAMPLE_LOOP_OFF,
 *                 \c SAMPLE_LOOP_UNI or \c SAMPLE_LOOP_BI.
 */
void Sample_set_loop(Sample* sample, Sample_loop loop);


/**
 * Gets the loop mode of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop mode.
 */
Sample_loop Sample_get_loop(Sample* sample);


/**
 * Sets the start point of the Sample loop.
 *
 * If the start point is set past the end of the loop or Sample, the loop is
 * turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param start    The loop start -- must not be \c NULL.
 */
void Sample_set_loop_start(Sample* sample, uint64_t start);


/**
 * Gets the start point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
uint64_t Sample_get_loop_start(Sample* sample);


/**
 * Sets the end point of the Sample loop.
 *
 * If the end point is set at or before the start point or past the length of
 * the Sample, the loop is turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param end      The loop end -- must not be \c NULL.
 */
void Sample_set_loop_end(Sample* sample, uint64_t end);


/**
 * Gets the end point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
uint64_t Sample_get_loop_end(Sample* sample);


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


