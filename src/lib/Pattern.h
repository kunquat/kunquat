

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


#ifndef K_PATTERN_H
#define K_PATTERN_H


#include <stdint.h>

#include <Playdata.h>
#include <Column.h>
#include <Reltime.h>
#include <Song_limits.h>


/**
 * This object contains a section of music.
 */
typedef struct Pattern
{
    Column* global;
    Column* cols[COLUMNS_MAX];
    Reltime length;
} Pattern;


/**
 * Creates a new Pattern object.
 * The caller shall eventually call del_Pattern() to destroy the Pattern
 * returned.
 *
 * \see del_Pattern()
 *
 * \return   The new Pattern object if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern* new_Pattern(void);


/**
 * Sets the length of the Pattern. No notes will be deleted.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL and must be
 *                 non-negative.
 */
void Pattern_set_length(Pattern* pat, Reltime* length);


/**
 * Gets the length of the Pattern.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 *
 * \return   The length -- must not be freed.
 */
Reltime* Pattern_get_length(Pattern* pat);


/**
 * Returns a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c COLUMNS_MAX.
 *
 * \return   The Column.
 */
Column* Pattern_get_col(Pattern* pat, int index);


/**
 * Gets the global Column of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 *
 * \return   The global Column.
 */
Column* Pattern_global(Pattern* pat);


/**
 * Mixes a portion of the Pattern. TODO: params
 *
 * \param pat       The Pattern -- must not be \c NULL.
 * \param nframes   The amount of frames to be mixed.
 * \param offset    The mixing buffer offset to be used -- must be
 *                  < \a nframes.
 * \param play      The Playdata object -- must not be \c NULL.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes. A value that is < \a nframes indicates that the
 *           mixing of the Pattern is complete.
 */
uint32_t Pattern_mix(Pattern* pat,
        uint32_t nframes,
        uint32_t offset,
        Playdata* play);


/**
 * Destroys an existing Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 */
void del_Pattern(Pattern* pat);


#endif // K_PATTERN_H


