

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <AAtree.h>
#include <player/Device_state.h>
#include <player/Device_states.h>
#include <memory.h>
#include <xassert.h>


struct Device_states
{
    AAtree* states;
};


Device_states* new_Device_states(void)
{
    Device_states* states = memory_alloc_item(Device_states);
    if (states == NULL)
        return NULL;

    states->states = NULL;

    states->states = new_AAtree(
            (int (*)(const void*, const void*))Device_state_cmp,
            (void (*)(void*))del_Device_state);
    if (states->states == NULL)
    {
        del_Device_states(states);
        return NULL;
    }

    return states;
}


Device_state* Device_states_get_state(
        const Device_states* states,
        uint32_t id)
{
    assert(states != NULL);
    assert(id > 0);

    const Device_state* key = DEVICE_STATE_KEY(id);
    assert(AAtree_contains(states->states, key));

    return AAtree_get_exact(states->states, key);
}


bool Device_states_set_audio_rate(Device_states* states, int32_t rate)
{
    assert(states != NULL);
    assert(rate > 0);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_set_audio_rate(ds, rate))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


bool Device_states_set_audio_buffer_size(Device_states* states, int32_t size)
{
    assert(states != NULL);
    assert(size >= 0);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_set_audio_buffer_size(ds, size))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


bool Device_states_allocate_space(Device_states* states, char* key)
{
    assert(states != NULL);
    assert(key != NULL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_allocate_space(ds, key))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


void Device_states_clear_audio_buffers(
        Device_states* states,
        uint32_t start,
        uint32_t stop)
{
    assert(states != NULL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_clear_audio_buffers(ds, start, stop);

        ds = AAiter_get_next(iter);
    }
}


void Device_states_reset(Device_states* states)
{
    assert(states != NULL);

    AAiter* iter = AAITER_AUTO;
    AAiter_change_tree(iter, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_reset(ds);

        ds = AAiter_get_next(iter);
    }
}


void del_Device_states(Device_states* states)
{
    if (states == NULL)
        return;

    del_AAtree(states->states);
    memory_free(states);
    return;
}


