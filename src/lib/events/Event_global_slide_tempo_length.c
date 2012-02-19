

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_global_slide_tempo_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


bool Event_global_slide_tempo_length_process(Playdata* global_state,
                                             Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TIMESTAMP)
    {
        return false;
    }
    if (global_state->tempo_slide != 0)
    {
        Reltime_init(&global_state->tempo_slide_int_left);
        Reltime_copy(&global_state->tempo_slide_left,
                     &value->value.Timestamp_type);
        double rems_total =
                (double)Reltime_get_beats(&value->value.Timestamp_type) *
                            KQT_RELTIME_BEAT +
                            Reltime_get_rem(&value->value.Timestamp_type);
        double slices = rems_total / 36756720; // slide updated 24 times per beat
        global_state->tempo_slide_update = (global_state->tempo_slide_target -
                                            global_state->tempo) / slices;
    }
    Reltime_copy(&global_state->tempo_slide_length,
                 &value->value.Timestamp_type);
    return true;
}


