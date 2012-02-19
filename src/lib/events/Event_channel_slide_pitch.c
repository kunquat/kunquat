

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
#include <math.h>
#include <float.h>

#include <Event_common.h>
#include <Event_channel_slide_pitch.h>
#include <Reltime.h>
#include <Voice.h>
#include <Scale.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>
#include <xmemory.h>


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SLIDE_PITCH,
                         slide_pitch);


bool Event_channel_slide_pitch_process(Channel_state* ch_state, Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        Event_check_voice(ch_state, i);
        Voice* voice = ch_state->fg[i];
        if (voice->gen->ins_params->pitch_locks[i].enabled)
        {
            continue;
        }
        Voice_state* vs = voice->state;
        pitch_t pitch = -1;
        if (voice->gen->ins_params->scale == NULL ||
                *voice->gen->ins_params->scale == NULL ||
                **voice->gen->ins_params->scale == NULL)
        {
            pitch = value->value.float_type;
        }
        else
        {
            pitch = Scale_get_pitch_from_cents(**voice->gen->ins_params->scale,
                                               value->value.float_type);
        }
        if (pitch <= 0)
        {
            continue;
        }
        if (Slider_in_progress(&vs->pitch_slider))
        {
            Slider_change_target(&vs->pitch_slider, pitch);
        }
        else
        {
            Slider_start(&vs->pitch_slider, pitch, vs->pitch);
        }
    }
    return true;
}


