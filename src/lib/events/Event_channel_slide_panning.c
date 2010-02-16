

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
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_channel_slide_panning.h>
#include <Reltime.h>
#include <Voice.h>

#include <xmemory.h>


static Event_field_desc slide_panning_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -1,
        .max.field.double_type = 1
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_channel_slide_panning,
                                   EVENT_CHANNEL_SLIDE_PANNING,
                                   double, target_panning)


static void Event_channel_slide_panning_process(Event_channel* event, Channel* ch);


Event_create_constructor(Event_channel_slide_panning,
                         EVENT_CHANNEL_SLIDE_PANNING,
                         slide_panning_desc,
                         event->target_panning = 0)


bool Event_channel_slide_panning_handle(Channel_state* ch_state, char* fields)
{
    assert(ch_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_panning_desc, data, state);
    if (state->error)
    {
        return false;
    }
    ch_state->panning_slide_target = data[0].field.double_type;
    ch_state->panning_slide_frames =
            Reltime_toframes(&ch_state->panning_slide_length,
                             *ch_state->tempo,
                             *ch_state->freq);
    double diff = ch_state->panning_slide_target - ch_state->panning;
    ch_state->panning_slide_update = diff / ch_state->panning_slide_frames;
    if (diff > 0)
    {
        ch_state->panning_slide = 1;
    }
    else if (diff < 0)
    {
        ch_state->panning_slide = -1;
    }
    else
    {
        ch_state->panning = ch_state->panning_slide_target;
        ch_state->panning_slide = 0;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice_state* vs = &ch_state->fg[i]->state.generic;
        vs->panning_slide_target = data[0].field.double_type;
        vs->panning_slide_frames = Reltime_toframes(&vs->panning_slide_length,
                                                    *ch_state->tempo,
                                                    *ch_state->freq);
        diff = vs->panning_slide_target - vs->panning;
        vs->panning_slide_update = diff / vs->panning_slide_frames;
        if (diff > 0)
        {
            vs->panning_slide = 1;
        }
        else if (diff < 0)
        {
            vs->panning_slide = -1;
        }
        else
        {
            vs->panning = vs->panning_slide_target;
            vs->panning_slide = 0;
        }
    }
    return true;
}


static void Event_channel_slide_panning_process(Event_channel* event, Channel* ch)
{
    (void)event;
    (void)ch;
    assert(false);
#if 0
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_SLIDE_PANNING);
    assert(voice != NULL);
    Event_channel_slide_panning* slide_panning = (Event_channel_slide_panning*)event;
    voice->state.generic.panning_slide_target = slide_panning->target_panning;
    voice->state.generic.panning_slide_frames =
            Reltime_toframes(&voice->state.generic.panning_slide_length,
                    voice->state.generic.tempo,
                    voice->state.generic.freq);
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    double diff = voice->state.generic.panning_slide_target -
            voice->state.generic.panning;
    voice->state.generic.panning_slide_update = diff /
            voice->state.generic.panning_slide_frames;
    if (diff > 0)
    {
        voice->state.generic.panning_slide = 1;
        ch_state->panning_slide = 1;
    }
    else if (diff < 0)
    {
        voice->state.generic.panning_slide = -1;
        ch_state->panning_slide = -1;
    }
    else
    {
        voice->state.generic.panning = voice->state.generic.panning_slide_target;
        voice->state.generic.panning_slide = 0;
        ch_state->panning = slide_panning->target_panning;
        ch_state->panning_slide = 0;
    }
    return;
#endif
}


