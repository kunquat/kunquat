

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
#include <stdio.h>
#include <string.h>

#include <Channel_state.h>
#include <Reltime.h>


Channel_state* Channel_state_init(Channel_state* state, int num, bool* mute)
{
    assert(state != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(mute != NULL);

    state->num = num;
    state->instrument = 0;
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

    return state;
}


Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(Channel_state));
#if 0
    dest->num = src->num;
    dest->mute = src->mute;

    dest->volume = src->volume;

    Reltime_copy(&dest->force_slide_length, &src->force_slide_length);
    dest->tremolo_length = src->tremolo_length;
    dest->tremolo_update = src->tremolo_update;
    dest->tremolo_depth = src->tremolo_depth;
    dest->tremolo_delay_update = src->tremolo_delay_update;

    Reltime_copy(&dest->pitch_slide_length, &src->pitch_slide_length);
    dest->vibrato_length = src->vibrato_length;
    dest->vibrato_update = src->vibrato_update;
    dest->vibrato_depth = src->vibrato_depth;
    dest->vibrato_delay_update = src->vibrato_delay_update;

    Reltime_copy(&dest->filter_slide_length, &src->filter_slide_length);
    dest->autowah_length = src->autowah_length;
    dest->autowah_update = src->autowah_update;
    dest->autowah_depth = src->autowah_depth;
    dest->autowah_delay_update = src->autowah_delay_update;

    dest->panning = src->panning;
    dest->panning_slide = src->panning_slide;
    Reltime_copy(&dest->panning_slide_length, &src->panning_slide_length);
    dest->panning_slide_target = src->panning_slide_target;
    dest->panning_slide_frames = src->panning_slide_frames;
    dest->panning_slide_update = src->panning_slide_update;
    dest->panning_slide_prog = src->panning_slide_prog;
#endif
    return dest;
}


