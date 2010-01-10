

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef PEAK_METER_H
#define PEAK_METER_H


#include <Mix_state.h>


/**
 * Writes a peak meter string suitable for Unicode terminal display.
 *
 * \param str         The string where the characters will be stored -- must
 *                    not be \c NULL.
 * \param len         The length of the string in Unicode characters -- must
 *                    be > \c 0.
 * \param mix_state   The Mix state -- must not be \c NULL.
 * \param lower       The lower bound of the dB scale -- must be finite and
 *                    < \ə upper.
 * \param upper       The upper bound of the dB scale -- must be finite and
 *                    > \a lower.
 * \param clipped     An array of clip counts -- must not be \c NULL.
 * \param unicode     Use Unicode characters for display.
 *
 * \return   The parameter \a str.
 */
char* get_peak_meter(char* str,
                     int len,
                     Mix_state* mix_state,
                     double lower,
                     double upper,
                     uint64_t* clipped,
                     bool unicode);


#endif // PEAK_METER_H


