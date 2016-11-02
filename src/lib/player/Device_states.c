

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Device_states.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <init/Connections.h>
#include <player/devices/Device_state.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


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
            (AAtree_item_cmp*)Device_state_cmp, (AAtree_item_destroy*)del_Device_state);
    if (states->states == NULL)
    {
        del_Device_states(states);
        return NULL;
    }

    return states;
}


bool Device_states_add_state(Device_states* states, Device_state* state)
{
    rassert(states != NULL);
    rassert(state != NULL);
    rassert(!AAtree_contains(states->states, state));

    if (!AAtree_ins(states->states, state))
        return false;

    Device_state_reset(state);

    return true;
}


Device_state* Device_states_get_state(const Device_states* states, uint32_t id)
{
    rassert(states != NULL);
    rassert(id > 0);

    const Device_state* key = DEVICE_STATE_KEY(id);
    rassert(AAtree_contains(states->states, key));

    return AAtree_get_exact(states->states, key);
}


void Device_states_remove_state(Device_states* states, uint32_t id)
{
    rassert(states != NULL);
    rassert(id > 0);

    const Device_state* key = DEVICE_STATE_KEY(id);
    del_Device_state(AAtree_remove(states->states, key));

    return;
}


bool Device_states_set_audio_rate(Device_states* states, int32_t rate)
{
    rassert(states != NULL);
    rassert(rate > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

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
    rassert(states != NULL);
    rassert(size >= 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_set_audio_buffer_size(ds, size))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}


/*
bool Device_states_allocate_space(Device_states* states, char* key)
{
    rassert(states != NULL);
    rassert(key != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        if (!Device_state_allocate_space(ds, key))
            return false;

        ds = AAiter_get_next(iter);
    }

    return true;
}
// */


void Device_states_clear_audio_buffers(
        Device_states* states, int32_t start, int32_t stop)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_clear_audio_buffers(ds, start, stop);

        ds = AAiter_get_next(iter);
    }

    return;
}


void Device_states_set_tempo(Device_states* states, double tempo)
{
    rassert(states != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_set_tempo(ds, tempo);
        ds = AAiter_get_next(iter);
    }

    return;
}


bool Device_states_prepare(Device_states* dstates, const Connections* conns)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);

    return Device_states_init_buffers(dstates, conns);
}


bool Device_states_init_buffers(Device_states* dstates, const Connections* conns)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    Device_states_reset_node_states(dstates);
    if (!Device_node_init_buffers_simple(master, dstates))
        return false;

    Device_states_reset_node_states(dstates);
    return Device_node_init_effect_buffers(master, dstates);
}


void Device_states_process_mixed_signals(
        Device_states* dstates,
        bool hack_reset,
        const Connections* conns,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo)
{
    rassert(dstates != NULL);
    rassert(conns != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(audio_rate > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Device_node* master = Connections_get_master(conns);
    rassert(master != NULL);
    if (buf_start >= buf_stop)
        return;

#if 0
    static bool called = false;
    if (!called)
    {
        Connections_print(graph, stderr);
    }
    called = true;
//    fprintf(stderr, "Mix process:\n");
#endif

    if (hack_reset)
        Device_states_reset_node_states(dstates);

    Device_node_process_mixed_signals(
            master, dstates, wbs, buf_start, buf_stop, audio_rate, tempo);

    return;
}


void Device_states_reset(Device_states* states)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_reset(ds);
        ds = AAiter_get_next(iter);
    }

    return;
}


void Device_states_reset_node_states(Device_states* states)
{
    rassert(states != NULL);

    AAiter* iter = AAiter_init(AAITER_AUTO, states->states);

    Device_state* ds = AAiter_get_at_least(iter, DEVICE_STATE_KEY(0));

    while (ds != NULL)
    {
        Device_state_set_node_state(ds, DEVICE_NODE_STATE_NEW);
        ds = AAiter_get_next(iter);
    }

    return;
}


void del_Device_states(Device_states* states)
{
    if (states == NULL)
        return;

    del_AAtree(states->states);
    memory_free(states);
    return;
}


