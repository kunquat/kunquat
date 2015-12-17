

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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

#include <Audio_buffer.h>
#include <Decl.h>
#include <devices/param_types/Sample_params.h>
#include <player/Proc_state.h>
#include <player/Voice_state.h>
#include <player/Work_buffer.h>


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
Sample* new_Sample_from_buffers(float* buffers[], int count, uint64_t length);


/**
 * Get the length of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The length in frames/buffer.
 */
uint64_t Sample_get_len(const Sample* sample);


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
 * Process Voice state with given Sample.
 *
 * \param sample        The Sample -- must not be \c NULL.
 * \param params        The Sample parameters -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param proc          The Processor -- must not be \c NULL.
 * \param proc_state    The Processor state -- must not be \c NULL.
 * \param wbs           The Work buffers -- must not be \c NULL.
 * \param out_buffer    The audio output buffer -- must not be \c NULL.
 * \param buf_start     The start index of the buffer area to be processed.
 * \param buf_stop      The stop index of the buffer area to be processed.
 * \param audio_rate    The audio rate -- must be positive.
 * \param tempo         The tempo -- must be > \c 0.
 * \param middle_tone   The frequency of the sound in the native speed of the
 *                      Sample -- must be > \c 0.
 * \param middle_freq   The mixing speed of the Sample used for playing
 *                      \a middle_tone -- must be > \c 0.
 * \param vol_scale     Volume scaling for this sample -- must be >= \c 0.
 */
uint32_t Sample_process_vstate(
        const Sample* sample,
        const Sample_params* params,
        Voice_state* vstate,
        const Processor* proc,
        const Proc_state* proc_state,
        const Work_buffers* wbs,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo,
        double middle_tone,
        double middle_freq,
        double vol_scale);


/**
 * Destroy a Sample.
 *
 * \param sample   The Sample, or \c NULL.
 */
void del_Sample(Sample* sample);


#endif // K_SAMPLE_H


