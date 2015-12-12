

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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>
#include <player/Channel_cv_state.h>
#include <player/Linear_controls.h>


typedef struct Entry
{
    char var_name[KQT_VAR_NAME_MAX];
    Value value;
    Linear_controls float_controls;
    bool carry;
    bool is_empty;
} Entry;


#define ENTRY_AUTO (&(Entry){ .var_name = "", .carry = false, .is_empty = true })


static int Entry_cmp(const Entry* e1, const Entry* e2)
{
    const int name_diff = strcmp(e1->var_name, e2->var_name);
    if (name_diff != 0)
        return name_diff;

    if (e1->value.type < e2->value.type)
        return -1;
    else if (e1->value.type > e2->value.type)
        return 1;

    return 0;
}


static Entry* Entry_init(Entry* entry, const char* var_name, Value_type var_type)
{
    assert(entry != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    strcpy(entry->var_name, var_name);

    if (var_type == VALUE_TYPE_BOOL)
    {
        entry->value.type = var_type;
        entry->value.value.bool_type = false;
    }
    else
    {
        Value* zero = VALUE_AUTO;
        zero->type = VALUE_TYPE_INT;
        zero->value.int_type = 0;
        Value_convert(&entry->value, zero, var_type);
    }

    if (var_type == VALUE_TYPE_FLOAT)
        Linear_controls_init(&entry->float_controls);

    entry->carry = false;
    entry->is_empty = true;

    return entry;
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
            (int (*)(const void*, const void*))Entry_cmp,
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

    const Entry* key = Entry_init(ENTRY_AUTO, "", VALUE_TYPE_BOOL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        if (entry->value.type == VALUE_TYPE_FLOAT)
            Linear_controls_set_audio_rate(&entry->float_controls, audio_rate);

        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_cv_state_set_tempo(Channel_cv_state* state, double tempo)
{
    assert(state != NULL);
    assert(tempo > 0);

    const Entry* key = Entry_init(ENTRY_AUTO, "", VALUE_TYPE_BOOL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        if (entry->value.type == VALUE_TYPE_FLOAT)
            Linear_controls_set_tempo(&entry->float_controls, tempo);

        entry = AAiter_get_next(iter);
    }

    return;
}


bool Channel_cv_state_add_entry(
        Channel_cv_state* state, const char* var_name, Value_type var_type)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* entry = Entry_init(ENTRY_AUTO, var_name, var_type);

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
    assert((value->type == VALUE_TYPE_BOOL) ||
            (value->type == VALUE_TYPE_INT) ||
            (value->type == VALUE_TYPE_FLOAT) ||
            (value->type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, value->type);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    if (value->type == VALUE_TYPE_FLOAT)
        Linear_controls_set_value(&entry->float_controls, value->value.float_type);

    Value_copy(&entry->value, value);

    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_slide_target_float(
        Channel_cv_state* state, const char* var_name, double value)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(value));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_slide_value_target(&entry->float_controls, value);
    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_slide_length_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_slide_value_length(&entry->float_controls, length);
    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_osc_speed_float(
        Channel_cv_state* state, const char* var_name, double speed)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(speed));
    assert(speed >= 0);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_osc_speed_value(&entry->float_controls, speed);
    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_osc_depth_float(
        Channel_cv_state* state, const char* var_name, double depth)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(isfinite(depth));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_osc_depth_value(&entry->float_controls, depth);
    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_osc_speed_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_osc_speed_slide_value(&entry->float_controls, length);
    entry->is_empty = false;

    return true;
}


bool Channel_cv_state_osc_depth_slide_float(
        Channel_cv_state* state, const char* var_name, const Tstamp* length)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(length != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);
    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    Linear_controls_osc_depth_slide_value(&entry->float_controls, length);
    entry->is_empty = false;

    return true;
}


const Value* Channel_cv_state_get_value(
        const Channel_cv_state* state, const char* var_name, Value_type var_type)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, var_type);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || entry->is_empty)
        return NULL;

    return &entry->value;
}


const Linear_controls* Channel_cv_state_get_float_controls(
        const Channel_cv_state* state, const char* var_name)
{
    assert(state != NULL);
    assert(var_name != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, VALUE_TYPE_FLOAT);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if ((entry == NULL) || entry->is_empty)
        return NULL;

    return &entry->float_controls;
}


bool Channel_cv_state_set_carrying_enabled(
        Channel_cv_state* state,
        const char* var_name,
        Value_type var_type,
        bool enabled)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, var_type);

    Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    entry->carry = enabled;

    return true;
}


bool Channel_cv_state_is_carrying_enabled(
        const Channel_cv_state* state, const char* var_name, Value_type var_type)
{
    assert(state != NULL);
    assert(var_name != NULL);
    assert(strlen(var_name) < KQT_VAR_NAME_MAX);
    assert((var_type == VALUE_TYPE_BOOL) ||
            (var_type == VALUE_TYPE_INT) ||
            (var_type == VALUE_TYPE_FLOAT) ||
            (var_type == VALUE_TYPE_TSTAMP));

    const Entry* key = Entry_init(ENTRY_AUTO, var_name, var_type);

    const Entry* entry = AAtree_get_exact(state->tree, key);
    if (entry == NULL)
        return false;

    return entry->carry;
}


void Channel_cv_state_update_float_controls(Channel_cv_state* state, uint64_t step_count)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, "", VALUE_TYPE_BOOL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        if (!entry->is_empty && (entry->value.type == VALUE_TYPE_FLOAT))
            Linear_controls_skip(&entry->float_controls, step_count);

        entry = AAiter_get_next(iter);
    }

    return;
}


void Channel_cv_state_reset(Channel_cv_state* state)
{
    assert(state != NULL);

    const Entry* key = Entry_init(ENTRY_AUTO, "", VALUE_TYPE_BOOL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, state->tree);

    Entry* entry = AAiter_get_at_least(iter, key);
    while (entry != NULL)
    {
        entry->is_empty = true;
        entry->carry = false;

        if (entry->value.type == VALUE_TYPE_FLOAT)
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


