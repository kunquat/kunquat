

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SAMPLE_H
#define K_SAMPLE_H


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <devices/generators/Sample_params.h>
#include <frame.h>


/**
 * Sample contains a digital sound sample.
 */
typedef struct Sample
{
//    Sample_params params;
//    char* path;           ///< The path of the file (if applicable -- otherwise NULL).
//    bool changed;         ///< Whether the sample (sound) data has changed after loading.
//    bool is_lossy;        ///< Whether this sample uses lossy compression.
    int channels;         ///< The number of channels (1 or 2).
    int bits;             ///< The bit resolution (8, 16, 24 or 32).
    bool is_float;        ///< Whether this sample is in floating point format.
    uint64_t len;         ///< The length of the sample (in amplitude values per channel).
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
 * Creates a new Sample from existing buffers.
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
Sample* new_Sample_from_buffers(float* buffers[], int count, uint64_t length);


/**
 * Copies Sample parameters into a Sample.
 *
 * This function copies all the fields except the format field.
 *
 * \param sample   The destination Sample -- must not be \c NULL.
 * \param params   The Sample parameters -- must not be \c NULL.
 */
//void Sample_set_params(Sample* sample, Sample_params* params);


/**
 * Gets the file format of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The Sample format.
 */
//Sample_format Sample_get_format(Sample* sample);


/**
 * Sets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param freq     The middle frequency -- must be > \c 0.
 */
//void Sample_set_freq(Sample* sample, double freq);


/**
 * Gets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The middle frequency.
 */
//double Sample_get_freq(Sample* sample);


/**
 * Gets the length of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The length in frames/buffer.
 */
uint64_t Sample_get_len(const Sample* sample);


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
//void Sample_set_loop(Sample* sample, Sample_loop loop);


/**
 * Gets the loop mode of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop mode.
 */
//Sample_loop Sample_get_loop(Sample* sample);


/**
 * Sets the start point of the Sample loop.
 *
 * If the start point is set past the end of the loop or Sample, the loop is
 * turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param start    The loop start -- must not be \c NULL.
 */
//void Sample_set_loop_start(Sample* sample, uint64_t start);


/**
 * Gets the start point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
//uint64_t Sample_get_loop_start(Sample* sample);


/**
 * Sets the end point of the Sample loop.
 *
 * If the end point is set at or before the start point or past the length of
 * the Sample, the loop is turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param end      The loop end -- must not be \c NULL.
 */
//void Sample_set_loop_end(Sample* sample, uint64_t end);


/**
 * Gets the end point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
//uint64_t Sample_get_loop_end(Sample* sample);


/**
 * Gets a buffer from the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param ch       The channel number -- must be >= \c 0 and less than the
 *                 number of channels in the Sample.
 *
 * \return   The buffer.
 */
void* Sample_get_buffer(Sample* sample, int ch);


/**
 * Destroys a Sample.
 *
 * \param sample   The Sample, or \c NULL.
 */
void del_Sample(Sample* sample);


#endif // K_SAMPLE_H


