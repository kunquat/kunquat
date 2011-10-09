

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SAMPLE_ENTRY_H
#define K_SAMPLE_ENTRY_H


#include <stdint.h>

#include <File_base.h>


typedef struct Sample_entry
{
    double ref_freq;  ///< The reference frequency in the mapping.
    /// The pitch offset of the middle frequency of this sample
    /// in the reference frequency.
    double cents;
    double vol_scale;
    uint16_t sample;
} Sample_entry;


/**
 * Parses a Sample entry from a string.
 *
 * \param entry   The Sample entry -- must not be \c NULL.
 * \param str     The string -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* Sample_entry_parse(Sample_entry* entry, char* str, Read_state* state);


#endif // K_SAMPLE_ENTRY_H


