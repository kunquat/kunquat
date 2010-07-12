

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
#include <stdio.h>
#include <string.h>

#include <Channel_state.h>
#include <Reltime.h>
#include <xassert.h>


bool Channel_state_init(Channel_state* state, int num, bool* mute)
{
    assert(state != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(mute != NULL);

    state->cgstate = new_Channel_gen_state();
    if (state->cgstate == NULL)
    {
        return false;
    }
    state->num = num;
    state->instrument = 0;
    state->generator = 0;
    state->dsp = 0;
    state->mute = mute;

    state->volume = 1;

    Reltime_set(&state->force_slide_length, 0, 0);
    state->tremolo_length = 0;
    state->tremolo_update = 0;
    state->tremolo_depth = 0;
    state->tremolo_delay_update = 1;

    Reltime_set(&state->pitch_slide_length, 0, 0);
    state->vibrato_length = 0;
    state->vibrato_update = 0;
    state->vibrato_depth = 0;
    state->vibrato_delay_update = 1;

    Reltime_set(&state->filter_slide_length, 0, 0);
    state->autowah_length = 0;
    state->autowah_update = 0;
    state->autowah_depth = 0;
    state->autowah_delay_update = 1;

    state->panning = 0;
    state->panning_slide = 0;
    Reltime_set(&state->panning_slide_length, 0, 0);
    state->panning_slide_target = 0;
    state->panning_slide_frames = 0;
    state->panning_slide_update = 0;
    state->panning_slide_prog = 0;

    return true;
}


Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(Channel_state));
    return dest;
}


void Channel_state_uninit(Channel_state* state)
{
    assert(state != NULL);
    if (state->cgstate != NULL)
    {
        del_Channel_gen_state(state->cgstate);
        state->cgstate = NULL;
    }
    return;
}


