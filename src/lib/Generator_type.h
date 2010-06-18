

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
    /// A simple pulse wave generator.
    GEN_TYPE_PULSE,
    /// A simple 303 square wave generator.
    GEN_TYPE_SQUARE303,
    /// A simple sawtooth wave generator.
    GEN_TYPE_SAWTOOTH,
    /// A sample-based type common in tracker programs.
    GEN_TYPE_PCM,
    /// A noise generator.
    GEN_TYPE_NOISE,
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


#endif // K_GENERATOR_TYPE_H


