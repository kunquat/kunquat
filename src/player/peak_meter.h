

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


#ifndef PEAK_METER_H
#define PEAK_METER_H


#include <kqt_Mix_state.h>


/**
 * Writes a peak meter string suitable for Unicode terminal display.
 *
 * \param str         The string where the characters will be stored -- must
 *                    not be \c NULL.
 * \param len         The length of the string in Unicode characters -- must
 *                    be > \c 0.
 * \param mix_state   The Kunquat mix state -- must not be \c NULL.
 * \param lower       The lower bound of the dB scale -- must be finite and
 *                    < \ə upper.
 * \param upper       The upper bound of the dB scale -- must be finite and
 *                    > \a lower.
 *
 * \return   The parameter \a str.
 */
char* get_peak_meter(char* str,
                     int len,
                     kqt_Mix_state* mix_state,
                     double lower,
                     double upper);


#endif // PEAK_METER_H


