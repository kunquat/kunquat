

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


#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_global_slide_tempo_length.h>
#include <Reltime.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc slide_tempo_length_desc[] =
{
    {
        .type = EVENT_FIELD_RELTIME,
        .min.field.Reltime_type = { 0, 0 },
        .max.field.Reltime_type = { INT64_MAX, KQT_RELTIME_BEAT - 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_reltime_and_get(Event_global_slide_tempo_length,
                                 EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                                 length);


Event_create_constructor(Event_global_slide_tempo_length,
                         EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                         slide_tempo_length_desc,
                         Reltime_set(&event->length, 0, 0));


bool Event_global_slide_tempo_length_process(Playdata* global_state,
                                             char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_tempo_length_desc, data, state);
    if (state->error)
    {
        return false;
    }
    if (global_state->tempo_slide != 0)
    {
        Reltime_init(&global_state->tempo_slide_int_left);
        Reltime_copy(&global_state->tempo_slide_left,
                     &data[0].field.Reltime_type);
        double rems_total =
                (double)Reltime_get_beats(&data[0].field.Reltime_type) *
                            KQT_RELTIME_BEAT +
                            Reltime_get_rem(&data[0].field.Reltime_type);
        double slices = rems_total / 36756720; // slide updated 24 times per beat
        global_state->tempo_slide_update = (global_state->tempo_slide_target -
                                            global_state->tempo) / slices;
    }
    Reltime_copy(&global_state->tempo_slide_length,
                 &data[0].field.Reltime_type);
    return true;
}


