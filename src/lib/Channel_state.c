

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <math.h>

#include <Channel_state.h>
#include <Environment.h>
#include <memory.h>
#include <Tstamp.h>
#include <xassert.h>


Channel_state* new_Channel_state(
        int num,
        Ins_table* insts,
        Environment* env,
        Voice_pool* voices,
        double* tempo,
        int32_t* audio_rate)
{
    assert(num >= 0);
    assert(num < KQT_CHANNELS_MAX);
    assert(insts != NULL);
    assert(env != NULL);
    assert(voices != NULL);
    assert(tempo != NULL);
    assert(audio_rate != NULL);

    Channel_state* ch_state = memory_alloc_item(Channel_state);
    if (ch_state == NULL)
        return NULL;

    if (!Channel_state_init(ch_state, num, env))
    {
        memory_free(ch_state);
        return NULL;
    }

    ch_state->insts = insts;
    ch_state->pool = voices;
    ch_state->tempo = tempo;
    ch_state->freq = audio_rate;

    return ch_state;
}


bool Channel_state_init(Channel_state* state, int num, Environment* env)
{
    assert(state != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(env != NULL);

    General_state_preinit(&state->parent);

    state->cgstate = new_Channel_gen_state();
    state->rand = new_Random();
    if (state->cgstate == NULL || state->rand == NULL ||
            !General_state_init(&state->parent, false, env))
    {
        del_Channel_gen_state(state->cgstate);
        del_Random(state->rand);
        return false;
    }

    char context[] = "chXX";
    snprintf(context, strlen(context) + 1, "ch%02x", num);
    Random_set_context(state->rand, context);
    state->event_cache = NULL;
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


void Channel_state_set_event_cache(Channel_state* state, Event_cache* cache)
{
    assert(state != NULL);
    assert(cache != NULL);
    del_Event_cache(state->event_cache);
    state->event_cache = cache;
    return;
}


void Channel_state_reset(Channel_state* state)
{
    assert(state != NULL);
    General_state_reset(&state->parent);

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        state->fg[i] = NULL;
        state->fg_id[i] = 0;
    }
    state->fg_count = 0;

    state->instrument = 0;
    state->generator = 0;
    state->effect = 0;
    state->inst_effects = false;
    state->dsp = 0;

    state->volume = 1;

    Tstamp_set(&state->force_slide_length, 0, 0);
    LFO_init(&state->tremolo, LFO_MODE_EXP);
    state->tremolo_speed = 0;
    Tstamp_init(&state->tremolo_speed_delay);
    state->tremolo_depth = 0;
    Tstamp_init(&state->tremolo_depth_delay);

    Tstamp_set(&state->pitch_slide_length, 0, 0);
    LFO_init(&state->vibrato, LFO_MODE_EXP);
    state->vibrato_speed = 0;
    Tstamp_init(&state->vibrato_speed_delay);
    state->vibrato_depth = 0;
    Tstamp_init(&state->vibrato_depth_delay);

    Tstamp_set(&state->filter_slide_length, 0, 0);
    LFO_init(&state->autowah, LFO_MODE_EXP);
    state->autowah_speed = 0;
    Tstamp_init(&state->autowah_speed_delay);
    state->autowah_depth = 0;
    Tstamp_init(&state->autowah_depth_delay);

    state->panning = 0;
    Slider_init(&state->panning_slider, SLIDE_MODE_LINEAR);

    state->arpeggio_ref = NAN;
    state->arpeggio_speed = 24;
    state->arpeggio_edit_pos = 1;
    state->arpeggio_tones[0] = state->arpeggio_tones[1] = NAN;

    Random_reset(state->rand);
    if (state->event_cache != NULL)
    {
        Event_cache_reset(state->event_cache);
    }
    return;
}


Channel_state* Channel_state_copy(Channel_state* dest, const Channel_state* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(Channel_state));
    return dest;
}


double Channel_state_get_fg_force(Channel_state* state, int gen_index)
{
    assert(state != NULL);
    assert(gen_index >= 0);
    assert(gen_index < KQT_GENERATORS_MAX);
    if (state->fg[gen_index] == NULL)
    {
        return NAN;
    }
    return Voice_get_actual_force(state->fg[gen_index]);
}


void Channel_state_uninit(Channel_state* state)
{
    if (state == NULL)
    {
        return;
    }
    del_Event_cache(state->event_cache);
    state->event_cache = NULL;
    del_Channel_gen_state(state->cgstate);
    state->cgstate = NULL;
    del_Random(state->rand);
    state->rand = NULL;
    General_state_deinit(&state->parent);
    return;
}


void del_Channel_state(Channel_state* state)
{
    if (state == NULL)
        return;

    Channel_state_uninit(state);
    memory_free(state);
    return;
}


