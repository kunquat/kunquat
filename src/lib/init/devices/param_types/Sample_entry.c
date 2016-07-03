

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


#include <init/devices/param_types/Sample_entry.h>

#include <debug/assert.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


bool Sample_entry_parse(Sample_entry* entry, Streader* sr)
{
    assert(entry != NULL);
    assert(sr != NULL);

    int64_t sample = -1;
    double sample_cents = NAN;
    double volume = NAN;

    if (!Streader_readf(sr, "[%i,%f,%f]", &sample, &sample_cents, &volume))
        return false;

    if (sample < 0)
    {
        Streader_set_error(sr, "Sample number must be non-negative");
        return false;
    }
    else if (sample >= 0x1000)
    {
        Streader_set_error(sr, "Sample number must be less than %d", 0x1000);
        return false;
    }

    if (!isfinite(sample_cents))
    {
        Streader_set_error(sr, "Sample cent offset is not finite");
        return false;
    }

    if (!isfinite(volume))
    {
        Streader_set_error(sr, "Volume adjustment is not finite");
        return false;
    }

    entry->sample = (int)sample;
    entry->cents = sample_cents;
    entry->vol_scale = exp2(volume / 6);

    return true;
}


