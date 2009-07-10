

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
#include <math.h>

#include <kunquat/Player_ext.h>
#include <kunquat/Reltime.h>

#include <Mix_state.h>


Mix_state* Mix_state_init(Mix_state* state)
{
    assert(state != NULL);
    state->playing = false;
    state->frames = 0;
    state->nanoseconds = 0;
    state->subsong = 0;
    state->section = 0;
    state->pattern = 0;
    kqt_Reltime_init(&state->pos);
    state->tempo = 0;
    state->voices = 0;
    for (int i = 0; i < 2; ++i)
    {
        state->min_amps[i] = INFINITY;
        state->max_amps[i] = -INFINITY;
        state->clipped[i] = 0;
    }
    return state;
}


Mix_state* Mix_state_copy(Mix_state* dest, Mix_state* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    dest->playing = src->playing;
    dest->frames = src->frames;
    dest->nanoseconds = src->nanoseconds;
    dest->subsong = src->subsong;
    dest->section = src->section;
    dest->pattern = src->pattern;
    kqt_Reltime_copy(&dest->pos, &src->pos);
    dest->tempo = src->tempo;
    dest->voices = src->voices;
    for (int i = 0; i < 2; ++i)
    {
        dest->min_amps[i] = src->min_amps[i];
        dest->max_amps[i] = src->max_amps[i];
        dest->clipped[i] = src->clipped[i];
    }
    return dest;
}


void Mix_state_from_context(Mix_state* mix_state, kqt_Context* context)
{
    assert(mix_state != NULL);
    assert(context != NULL);
    mix_state->playing = !kqt_Context_end_reached(context);
    mix_state->frames = kqt_Context_get_frames_mixed(context);
    mix_state->nanoseconds = kqt_Context_get_position_ns(context);
    char* pos = kqt_Context_get_position(context);
    long long beats = 0;
    long rem = 0;
    kqt_unwrap_time(pos, &mix_state->subsong, &mix_state->section, &beats, &rem, NULL);
/*    mix_state->pattern = kqt_Context_get_pattern_index(context,
                                                       mix_state->subsong,
                                                       mix_state->section); */
    kqt_Reltime_set(&mix_state->pos, beats, rem);
    mix_state->tempo = kqt_Context_get_tempo(context);
    mix_state->voices = kqt_Context_get_voice_count(context);
    for (int i = 0; i < 2; ++i)
    {
        mix_state->min_amps[i] = kqt_Context_get_min_amplitude(context, i);
        mix_state->max_amps[i] = kqt_Context_get_max_amplitude(context, i);
        mix_state->clipped[i] = kqt_Context_get_clipped(context, i);
    }
    kqt_Context_reset_stats(context);
    return;
}


