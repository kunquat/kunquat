

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Channel_stream_state.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/Linear_controls.h>
#include <string/var_name.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char name[KQT_VAR_NAME_MAX + 1];

    Linear_controls controls;
    bool is_set;

    Tstamp slide_length;
    double osc_speed;
    Tstamp osc_speed_slide;
    double osc_depth;
    Tstamp osc_depth_slide;
    bool carry;
} Entry;


struct Channel_stream_state
{
    AAtree* tree;
};


Channel_stream_state* new_Channel_stream_state(void)
{
    Channel_stream_state* state = memory_alloc_item(Channel_stream_state);
    if (state == NULL)
        return NULL;

    state->tree = NULL;

    state->tree = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)memory_free);
    if (state->tree == NULL)
    {
        del_Channel_stream_state(state);
        return NULL;
    }

    return state;
}


void Channel_stream_state_set_audio_rate(
        Channel_stream_state* state, int32_t audio_rate)
{
    rassert(state != NULL);
    rassert(audio_rate > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, state->tree);

    Entry* entry = AAiter_get_at_least(iter, "");
    while (entry != NULL)
    {
        Linear_controls_set_audio_rate(&entry->controls, audio_rate);
        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_stream_state_set_tempo(Channel_stream_state* state, double tempo)
{
    rassert(state != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, state->tree);

    Entry* entry = AAiter_get_at_least(iter, "");
    while (entry != NULL)
    {
        Linear_controls_set_tempo(&entry->controls, tempo);
        entry = AAiter_get_next(iter);
    }

    return;
}


bool Channel_stream_state_add_entry(
        Channel_stream_state* state, const char* stream_name)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));

    if (!AAtree_contains(state->tree, stream_name))
    {
        Entry* new_entry = memory_alloc_item(Entry);
        if (new_entry == NULL)
            return false;

        strcpy(new_entry->name, stream_name);
        Linear_controls_init(&new_entry->controls);
        new_entry->is_set = false;

        Tstamp_set(&new_entry->slide_length, -1, 0);
        new_entry->osc_speed = NAN;
        Tstamp_set(&new_entry->osc_speed_slide, -1, 0);
        new_entry->osc_depth = NAN;
        Tstamp_set(&new_entry->osc_depth_slide, -1, 0);
        new_entry->carry = false;

        if (!AAtree_ins(state->tree, new_entry))
        {
            memory_free(new_entry);
            return false;
        }
    }

    return true;
}


bool Channel_stream_state_set_value(
        Channel_stream_state* state, const char* stream_name, double value)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(isfinite(value));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_set_value(&entry->controls, value);
    entry->is_set = true;

    return true;
}


bool Channel_stream_state_slide_target(
        Channel_stream_state* state, const char* stream_name, double value)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(isfinite(value));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_slide_value_target(&entry->controls, value);

    return true;
}


bool Channel_stream_state_slide_length(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(length != NULL);

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_slide_value_length(&entry->controls, length);
    Tstamp_copy(&entry->slide_length, length);

    return true;
}


bool Channel_stream_state_set_osc_speed(
        Channel_stream_state* state, const char* stream_name, double speed)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(isfinite(speed));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_osc_speed_value(&entry->controls, speed);
    entry->osc_speed = speed;

    return true;
}


bool Channel_stream_state_set_osc_depth(
        Channel_stream_state* state, const char* stream_name, double depth)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(isfinite(depth));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_osc_depth_value(&entry->controls, depth);
    entry->osc_depth = depth;

    return true;
}


bool Channel_stream_state_set_osc_speed_slide(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(length != NULL);

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_osc_speed_slide_value(&entry->controls, length);
    Tstamp_copy(&entry->osc_speed_slide, length);

    return true;
}


bool Channel_stream_state_set_osc_depth_slide(
        Channel_stream_state* state, const char* stream_name, const Tstamp* length)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(length != NULL);

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_osc_depth_slide_value(&entry->controls, length);
    Tstamp_copy(&entry->osc_depth_slide, length);

    return true;
}


bool Channel_stream_state_set_controls(
        Channel_stream_state* state,
        const char* stream_name,
        const Linear_controls* controls)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(controls != NULL);

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    Linear_controls_copy(&entry->controls, controls);
    entry->is_set = true;

    return true;
}


const Linear_controls* Channel_stream_state_get_controls(
        const Channel_stream_state* state, const char* stream_name)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));

    const Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return NULL;

    return &entry->controls;
}


bool Channel_stream_state_set_carrying_enabled(
        Channel_stream_state* state, const char* stream_name, bool enabled)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    entry->carry = enabled;

    return true;
}


bool Channel_stream_state_is_carrying_enabled(
        const Channel_stream_state* state, const char* stream_name)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    return entry->carry;
}


bool Channel_stream_state_apply_overrides(
        const Channel_stream_state* state,
        const char* stream_name,
        Linear_controls* controls)
{
    rassert(state != NULL);
    rassert(stream_name != NULL);
    rassert(is_valid_var_name(stream_name));
    rassert(controls != NULL);

    Entry* entry = AAtree_get_exact(state->tree, stream_name);
    if (entry == NULL)
        return false;

    if (Tstamp_get_beats(&entry->slide_length) >= 0)
        Linear_controls_slide_value_length(controls, &entry->slide_length);
    if (!isnan(entry->osc_speed))
        Linear_controls_set_osc_speed_default_value(controls, entry->osc_speed);
    if (Tstamp_get_beats(&entry->osc_speed_slide) >= 0)
        Linear_controls_osc_speed_slide_value(controls, &entry->osc_speed_slide);
    if (!isnan(entry->osc_depth))
        Linear_controls_set_osc_depth_default_value(controls, entry->osc_depth);
    if (Tstamp_get_beats(&entry->osc_depth_slide) >= 0)
        Linear_controls_osc_depth_slide_value(controls, &entry->osc_depth_slide);

    return true;
}


void Channel_stream_state_update(Channel_stream_state* state, int64_t step_count)
{
    rassert(state != NULL);
    rassert(step_count >= 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, state->tree);

    Entry* entry = AAiter_get_at_least(iter, "");
    while (entry != NULL)
    {
        if (entry->is_set && !isnan(Linear_controls_get_value(&entry->controls)))
            Linear_controls_skip(&entry->controls, step_count);

        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_stream_state_reset(Channel_stream_state* state)
{
    rassert(state != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, state->tree);

    Entry* entry = AAiter_get_at_least(iter, "");
    while (entry != NULL)
    {
        Linear_controls_init(&entry->controls);
        entry->is_set = false;

        Tstamp_set(&entry->slide_length, -1, 0);
        entry->osc_speed = NAN;
        Tstamp_set(&entry->osc_speed_slide, -1, 0);
        entry->osc_depth = NAN;
        Tstamp_set(&entry->osc_depth_slide, -1, 0);
        entry->carry = false;

        entry = AAiter_get_next(iter);
    }

    return;
}


void del_Channel_stream_state(Channel_stream_state* state)
{
    if (state == NULL)
        return;

    del_AAtree(state->tree);
    memory_free(state);

    return;
}


