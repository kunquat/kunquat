

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


#ifndef STATUS_LINE_H
#define STATUS_LINE_H


#include <stdbool.h>
#include <stdint.h>

#include <kunquat/Mix_state.h>


/**
 * Writes a status line.
 *
 * \param line         The string where the line will be written -- must not
 *                     be \c NULL.
 * \param max_len      The maximum length of \a line.
 * \param mix_state    The Kunquat mix state -- must not be \c NULL.
 * \param min_len      Minimum length of \a line -- must be < \a max_len.
 * \param clipped      An array of clip counts -- must not be \c NULL.
 * \param ns_total     The length of the playback in frames.
 * \param max_voices   The maximum number of Voices used.
 * \param freq         The mixing frequency -- must be > \c 0.
 * \param unicode      Use Unicode characters for display.
 *
 * \return   The new minimum line length. This is always >= \a min_len.
 */
int get_status_line(char* line,
                    int max_len,
                    kqt_Mix_state* mix_state,
                    int min_len,
                    uint64_t* clipped,
                    long long ns_total,
                    uint16_t max_voices,
                    uint32_t freq,
                    bool unicode);


#endif // STATUS_LINE_H


