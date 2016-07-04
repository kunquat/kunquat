

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_SAMPLE_ENTRY_H
#define KQT_SAMPLE_ENTRY_H


#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>


typedef struct Sample_entry
{
    double ref_freq;  ///< The reference frequency in the mapping.
    /// The pitch offset of the middle frequency of this sample
    /// in the reference frequency.
    int sample;
    double cents;
    double vol_scale;
} Sample_entry;


/**
 * Parse a Sample entry from a string.
 *
 * \param entry   The Sample entry -- must not be \c NULL.
 * \param sr      The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Sample_entry_parse(Sample_entry* entry, Streader* sr);


#endif // KQT_SAMPLE_ENTRY_H


