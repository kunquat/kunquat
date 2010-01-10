

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <kunquat/Player_ext.h>

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
    state->beat = 0;
    state->beat_rem = 0;
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
    dest->beat = src->beat;
    dest->beat_rem = src->beat_rem;
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


void Mix_state_from_handle(Mix_state* mix_state, kqt_Handle* handle)
{
    assert(mix_state != NULL);
    assert(handle != NULL);
    mix_state->playing = !kqt_Handle_end_reached(handle);
    mix_state->frames = kqt_Handle_get_frames_mixed(handle);
    mix_state->nanoseconds = kqt_Handle_get_position(handle);
    char* pos = kqt_Handle_get_position_desc(handle);
    kqt_unwrap_time(pos, &mix_state->subsong, &mix_state->section,
            &mix_state->beat, &mix_state->beat_rem, NULL);
/*    mix_state->pattern = kqt_Handle_get_pattern_index(handle,
                                                       mix_state->subsong,
                                                       mix_state->section); */
    mix_state->tempo = kqt_Handle_get_tempo(handle);
    mix_state->voices = kqt_Handle_get_voice_count(handle);
    for (int i = 0; i < 2; ++i)
    {
        mix_state->min_amps[i] = kqt_Handle_get_min_amplitude(handle, i);
        mix_state->max_amps[i] = kqt_Handle_get_max_amplitude(handle, i);
        mix_state->clipped[i] = kqt_Handle_get_clipped(handle, i);
    }
    kqt_Handle_reset_stats(handle);
    return;
}


