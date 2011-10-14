

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


#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <File_base.h>
#include <Sample_entry.h>
#include <xassert.h>


char* Sample_entry_parse(Sample_entry* entry, char* str, Read_state* state)
{
    assert(entry != NULL);
    assert(str != NULL);
    assert(state != NULL);
    str = read_const_char(str, '[', state);
    double sample_cents = NAN;
    str = read_double(str, &sample_cents, state);
    str = read_const_char(str, ',', state);
    double volume = NAN;
    str = read_double(str, &volume, state);
    str = read_const_char(str, ',', state);
    int64_t sample = -1;
    str = read_int(str, &sample, state);
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        return str;
    }
    if (!isfinite(sample_cents))
    {
        Read_state_set_error(state,
                "Sample cent offset is not finite");
        return str;
    }
    if (!isfinite(volume))
    {
        Read_state_set_error(state,
                "Volume adjustment is not finite");
        return str;
    }
    if (sample < 0)
    {
        Read_state_set_error(state,
                "Sample number must be non-negative");
        return str;
    }
    entry->cents = sample_cents;
    entry->vol_scale = exp2(volume / 6);
    entry->sample = sample;
    return str;
}


