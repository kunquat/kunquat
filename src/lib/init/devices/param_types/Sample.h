

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_SAMPLE_H
#define KQT_SAMPLE_H


#include <decl.h>
#include <init/devices/param_types/Sample_params.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Sample contains a digital sound sample.
 */
struct Sample
{
//    Sample_params params;
//    char* path;           ///< The path of the file (if applicable -- otherwise NULL).
//    bool changed;         ///< Whether the sample (sound) data has changed after loading.
//    bool is_lossy;        ///< Whether this sample uses lossy compression.
    int channels;         ///< The number of channels (1 or 2).
    int bits;             ///< The bit resolution (8, 16, 24 or 32).
    bool is_float;        ///< Whether this sample is in floating point format.
    int64_t len;          ///< The length of the sample (in amplitude values per channel).
    void* data[2];        ///< The sample data.
};


/**
 * Create a new Sample.
 *
 * \return   The new Sample if successful, or \c NULL if memory allocation
 *           failed.
 */
Sample* new_Sample(void);


/**
 * Create a new Sample from existing buffers.
 *
 * \param buffers   The buffers -- must not be \c NULL. The Sample will
 *                  assume ownership of the buffers (but not the actual
 *                  array parameter) and free them when it is destroyed.
 * \param count     The number of buffers in \a buffers -- must be \c 1 or
 *                  \c 2.
 * \param length    The length of each buffer in \a buffers measured in
 *                  amplitude values -- must be > \c 0 and must not exceed
 *                  the real length.
 *
 * \return   The new Sample if successful, or \c NULL if memory allocation
 *           failed.
 */
Sample* new_Sample_from_buffers(float* buffers[], int count, int64_t length);


/**
 * Get the length of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The length in frames/buffer.
 */
int64_t Sample_get_len(const Sample* sample);


/**
 * Get a buffer from the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param ch       The channel number -- must be >= \c 0 and less than the
 *                 number of channels in the Sample.
 *
 * \return   The buffer.
 */
void* Sample_get_buffer(Sample* sample, int ch);


/**
 * Destroy a Sample.
 *
 * \param sample   The Sample, or \c NULL.
 */
void del_Sample(Sample* sample);


#endif // KQT_SAMPLE_H


