

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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


#ifndef K_GENERATOR_TYPE_H
#define K_GENERATOR_TYPE_H


#include <File_base.h>


typedef enum
{
    /// Not a valid type.
    GEN_TYPE_NONE = 0,
    /// A type used for debugging.
    /// Output is a narrow pulse wave (with one sample value 1, the rest are
    /// 0.5) that lasts no more than 10 phase cycles. Note Off lasts no more
    /// than two phase cycles with all sample values negated.
    GEN_TYPE_DEBUG,
    /// A simple sine wave instrument for testing by ear.
    GEN_TYPE_SINE,
    /// A simple triangle wave generator.
    GEN_TYPE_TRIANGLE,
    /// A simple square wave generator.
    GEN_TYPE_SQUARE,
    /// A simple 303 square wave generator.
    GEN_TYPE_SQUARE303,
    /// A simple sawtooth wave generator.
    GEN_TYPE_SAWTOOTH,
    /// A sample-based type common in tracker programs.
    GEN_TYPE_PCM,
    /// A type for reading audio data from disk -- used for large audio files.
    GEN_TYPE_PCM_DISK,
    /// An implementation of Paul Nasca's PADsynth algorithm.
    GEN_TYPE_PADSYNTH,
    /// Sentinel -- never used as a valid type.
    GEN_TYPE_LAST
} Gen_type;


/**
 * Parses a Generator type description.
 *
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The Generator type if successful, \c GEN_TYPE_NONE if \a str is
 *           \c NULL, or \c GEN_TYPE_LAST if an error occurred.
 */
Gen_type Generator_type_parse(char* str, Read_state* state);


/**
 * Tells whether the given subkey is part of the specification of given type.
 *
 * \param type     The type -- must be a valid type.
 * \param subkey   The subkey. This is the part after "generator_XX/".
 *
 * \return   \c true if and only if \a subkey is part of the specification of
 *           \a type.
 */
bool Generator_type_has_subkey(Gen_type type, const char* subkey);


#endif // K_GENERATOR_TYPE_H


