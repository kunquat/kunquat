

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_FILTER_UTILS_H
#define KQT_FILTER_UTILS_H


#include <decl.h>

#include <stdint.h>


/**
 * Convert parameter cutoff buffer to values expected by the filter implementation.
 *
 * \param dest          The destination buffer -- must not be \c NULL.
 * \param src           The source buffer, or \c NULL.
 * \param def_cutoff    Default cutoff value -- must be finite.
 * \param frame_count   Number of frames to be processed -- must be > \c 0.
 * \param audio_rate    The audio rate -- must be > \c 0.
 */
void transform_cutoff(
        Work_buffer* dest,
        const Work_buffer* src,
        double def_cutoff,
        int32_t frame_count,
        int32_t audio_rate);


#endif // KQT_FILTER_UTILS_H


