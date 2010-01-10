

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


#ifndef STATUS_LINE_H
#define STATUS_LINE_H


#include <stdbool.h>
#include <stdint.h>

#include <Mix_state.h>


/**
 * Writes a status line.
 *
 * \param line         The string where the line will be written -- must not
 *                     be \c NULL.
 * \param max_len      The maximum length of \a line.
 * \param mix_state    The Mix state -- must not be \c NULL.
 * \param min_len      Minimum length of \a line -- must be < \a max_len.
 * \param clipped      An array of clip counts -- must not be \c NULL.
 * \param ns_total     The length of the playback in frames.
 * \param max_voices   The maximum number of Voices used.
 * \param unicode      Use Unicode characters for display.
 *
 * \return   The new minimum line length. This is always >= \a min_len.
 */
int get_status_line(char* line,
                    int max_len,
                    Mix_state* mix_state,
                    int min_len,
                    uint64_t* clipped,
                    long long ns_total,
                    uint16_t max_voices,
                    bool unicode);


#endif // STATUS_LINE_H


