

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


#ifndef KQT_SAMPLE_PARAMS_H
#define KQT_SAMPLE_PARAMS_H


#include <decl.h>
#include <string/Streader.h>

#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    /// Uninitialised.
    SAMPLE_FORMAT_NONE = 0,
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
 * Common parameters for a Sample.
 */
struct Sample_params
{
    Sample_format format; ///< The file format.
    double mid_freq;      ///< The playback frequency used to represent 440 Hz tone.
    Sample_loop loop;     ///< Loop setting.
    uint64_t loop_start;  ///< Loop start.
    uint64_t loop_end;    ///< Loop end (the frame at this index will not be played).
};


/**
 * Initialise Sample parameters.
 *
 * \param params   The Sample parameters -- must not be \c NULL.
 *
 * \return   The parameter \a params.
 */
Sample_params* Sample_params_init(Sample_params* params);


/**
 * Parse the Sample parameters from a string.
 *
 * \param params   The Sample parameters -- must not be \c NULL.
 * \param sr       The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Sample_params_parse(Sample_params* params, Streader* sr);


/**
 * Copy Sample parameters.
 *
 * \param dest   The copy destination -- must not be \c NULL.
 * \param src    The copy source -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Sample_params* Sample_params_copy(Sample_params* dest, const Sample_params* src);


#endif // KQT_SAMPLE_PARAMS_H


