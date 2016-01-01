

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Channel_cv_state.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/Linear_controls.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char var_name[KQT_VAR_NAME_MAX];

    Value value;
    Linear_controls float_controls;

    bool is_set;
    bool carry;
} Entry;


#define ENTRY_AUTO (&(Entry){ .var_name = "", .is_set = false, .carry = false })


static Entry* Entry_init(Entry* entry, const char* var_name)
{
    assert(entry != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    strcpy(entry->var_name, var_name);

    entry->value.type = VALUE_TYPE_NONE;
    Linear_controls_init(&entry->float_controls);

    entry->is_set = false;
    entry->carry = false;

    return entry;
}


static bool Entry_contains_float(const Entry* entry)
{
    assert(entry != NULL);
    return (entry->is_set && !isnan(Linear_controls_get_value(&entry->float_controls)));
}


struct Channel_cv_state
{
    AAtree* tree;
};


Channel_cv_state* new_Channel_cv_state(void)
{
    Channel_cv_state* state = memory_alloc_item(Channel_cv_state);
    if (state == NULL)
        return NULL;

    state->tree = NULL;

    state->tree = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))memory_free);
    if (state->tree == NULL)
    {
        del_Channel_cv_state(state);
        return NULL;
    }

    return state;
}


void Channel_cv_state_set_audio_rate(Channel_cv_state* state, int32_t audio_rate)
{
    assert(state != NULL);
    assert(audio_rate > 0);

    const Entry* key = Entry_init(ENTRY_AUTO, "");

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        Linear_controls_set_audio_rate(&entry->float_controls, audio_rate);
        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_cv_state_set_tempo(Channel_cv_state* state, double tempo)
{
    assert(state != NULL);
    assert(tempo > 0);

    const Entry* key = Entry_init(ENTRY_AUTO, "");

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        Linear_controls_set_tempo(&entry->float_controls, tempo);
        entry = AAiter_get_next(iter);
    }

    return;
}


bool Channel_cv_state_add_entry(Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* entry = Entry_init(ENTRY_AUTO, var_name);

    if (!AAtree_contains(state->tree, entry))
    {
        Entry* new_entry = memory_alloc_item(Entry);
        if (new_entry == NULL)
            return false;

        *new_entry = *entry;

        if (!AAtree_ins(state->tree, new_entry))
        {
            memory_free(new_entry);
            return false;
        }
    }

    return true;
}


bool Channel_cv_state_set_value(
        Channel_cv_state* state, const char* var_name, const Value* value)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert(value != NULL);
    assert(Value_type_is_realtime(value->type));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Value_copy(&entry->value, value);

    Value* fvalue = VALUE_AUTO;
    if (Value_convert(fvalue, value, VALUE_TYPE_FLOAT))
        Linear_controls_set_value(&entry->float_controls, fvalue->value.float_type);
    else
        Linear_controls_init(&entry->float_controls);

    entry->is_set = true;

    return true;
}


bool Channel_cv_state_slide_target_float(
        Channel_cv_state* state, const char* var_name, double value)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(value));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_slide_value_target(&entry->float_controls, value);

    return true;
}


bool Channel_cv_state_slide_length_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_slide_value_length(&entry->float_controls, length);

    return true;
}


bool Channel_cv_state_osc_speed_float(
        Channel_cv_state* state, const char* var_name, double speed)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(speed));
    assert(speed >= 0);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_osc_speed_value(&entry->float_controls, speed);

    return true;
}


bool Channel_cv_state_osc_depth_float(
        Channel_cv_state* state, const char* var_name, double depth)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(depth));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_osc_depth_value(&entry->float_controls, depth);

    return true;
}


bool Channel_cv_state_osc_speed_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_osc_speed_slide_value(&entry->float_controls, length);

    return true;
}


bool Channel_cv_state_osc_depth_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return false;

    Linear_controls_osc_depth_slide_value(&entry->float_controls, length);

    return true;
}


const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !entry->is_set)
        return NULL;

    return &entry->value;
}


const Linear_controls* Channel_cv_state_get_float_controls(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || !Entry_contains_float(entry))
        return NULL;

    return &entry->float_controls;
}


bool Channel_cv_state_set_carrying_enabled(
        Channel_cv_state* state, const char* var_name, bool enabled)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    entry->carry = enabled;

    return true;
}


bool Channel_cv_state_is_carrying_enabled(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    return entry->carry;
}


void Channel_cv_state_update_float_controls(Channel_cv_state* state, uint64_t step_count)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, "");

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        if (Entry_contains_float(entry))
            Linear_controls_skip(&entry->float_controls, step_count);

        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_cv_state_reset(Channel_cv_state* state)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, "");

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        entry->is_set = false;
        entry->carry = false;

        Linear_controls_init(&entry->float_controls);

        entry = AAiter_get_next(iter);
    }

    return;
}


void del_Channel_cv_state(Channel_cv_state* state)
{
    if (state == NULL)
        return;

    del_AAtree(state->tree);
    memory_free(state);

    return;
}


