

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <Channel_state.h>


Channel_state* Channel_state_init(Channel_state* state, int num, bool* mute)
{
    assert(state != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(mute != NULL);

    state->num = num;
    state->mute = mute;

    state->volume = 1;

    state->tremolo_length = 0;
    state->tremolo_update = 0;
    state->tremolo_depth = 0;
    state->tremolo_delay_update = 1;

    state->vibrato_length = 0;
    state->vibrato_update = 0;
    state->vibrato_depth = 0;
    state->vibrato_delay_update = 1;

    state->autowah_length = 0;
    state->autowah_update = 0;
    state->autowah_depth = 0;
    state->autowah_delay_update = 1;

    state->panning = 0;
    state->panning_slide = 0;
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

    dest->num = src->num;
    dest->mute = src->mute;

    dest->volume = src->volume;

    dest->tremolo_length = src->tremolo_length;
    dest->tremolo_update = src->tremolo_update;
    dest->tremolo_depth = src->tremolo_depth;
    dest->tremolo_delay_update = src->tremolo_delay_update;

    dest->vibrato_length = src->vibrato_length;
    dest->vibrato_update = src->vibrato_update;
    dest->vibrato_depth = src->vibrato_depth;
    dest->vibrato_delay_update = src->vibrato_delay_update;

    dest->autowah_length = src->autowah_length;
    dest->autowah_update = src->autowah_update;
    dest->autowah_depth = src->autowah_depth;
    dest->autowah_delay_update = src->autowah_delay_update;

    dest->panning = src->panning;
    dest->panning_slide = src->panning_slide;
    dest->panning_slide_target = src->panning_slide_target;
    dest->panning_slide_frames = src->panning_slide_frames;
    dest->panning_slide_update = src->panning_slide_update;
    dest->panning_slide_prog = src->panning_slide_prog;

    return dest;
}


