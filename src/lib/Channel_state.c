

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <Environment.h>
#include <Reltime.h>
#include <xassert.h>


bool Channel_state_init(Channel_state* state, int num, bool* mute,
                        Environment* env)
{
    assert(state != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(mute != NULL);
    assert(env != NULL);
    General_state_init(&state->parent, false, env);

    state->cgstate = new_Channel_gen_state();
    state->rand = new_Random();
    if (state->cgstate == NULL || state->rand == NULL)
    {
        del_Channel_gen_state(state->cgstate);
        del_Random(state->rand);
        return false;
    }
    char context[] = "chXX";
    snprintf(context, strlen(context) + 1, "ch%02x", num);
    Random_set_context(state->rand, context);
    state->mute = mute;
    state->num = num;
    Channel_state_reset(state);
    return true;
}


void Channel_state_set_random_seed(Channel_state* state, uint64_t seed)
{
    assert(state != NULL);
    Random_set_seed(state->rand, seed);
    return;
}


void Channel_state_reset(Channel_state* state)
{
    assert(state != NULL);
    General_state_reset(&state->parent);
    state->instrument = 0;
    state->generator = 0;
    state->effect = 0;
    state->inst_effects = false;
    state->dsp = 0;

    state->volume = 1;

    Reltime_set(&state->force_slide_length, 0, 0);
    LFO_init(&state->tremolo, LFO_MODE_EXP);
    state->tremolo_speed = 0;
    Reltime_init(&state->tremolo_speed_delay);
    state->tremolo_depth = 0;
    Reltime_init(&state->tremolo_depth_delay);

    Reltime_set(&state->pitch_slide_length, 0, 0);
    LFO_init(&state->vibrato, LFO_MODE_EXP);
    state->vibrato_speed = 0;
    Reltime_init(&state->vibrato_speed_delay);
    state->vibrato_depth = 0;
    Reltime_init(&state->vibrato_depth_delay);

    Reltime_set(&state->filter_slide_length, 0, 0);
    LFO_init(&state->autowah, LFO_MODE_EXP);
    state->autowah_speed = 0;
    Reltime_init(&state->autowah_speed_delay);
    state->autowah_depth = 0;
    Reltime_init(&state->autowah_depth_delay);

    state->panning = 0;
    Slider_init(&state->panning_slider, SLIDE_MODE_LINEAR);

    Random_reset(state->rand);
    return;
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
    if (state == NULL)
    {
        return;
    }
    del_Channel_gen_state(state->cgstate);
    del_Random(state->rand);
    state->cgstate = NULL;
    return;
}


