

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Au_state.h>

#include <debug/assert.h>
#include <init/Connections.h>
#include <init/devices/Audio_unit.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/devices/Device_state.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


static Device_state_reset_func Au_state_reset;


static void Au_state_init(
        Au_state* au_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(au_state != NULL);

    Device_state_init(&au_state->parent, device, audio_rate, audio_buffer_size);
    au_state->parent.reset = Au_state_reset;

    au_state->dstates = NULL;

    Au_state_reset(&au_state->parent);

    return;
}


static void mix_interface_connection(
        Device_state* out_ds,
        const Device_state* in_ds,
        uint32_t buf_start,
        uint32_t buf_stop)
{
    assert(out_ds != NULL);
    assert(in_ds != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Audio_buffer* out = Device_state_get_audio_buffer(
                out_ds, DEVICE_PORT_TYPE_SEND, port);
        const Audio_buffer* in = Device_state_get_audio_buffer(
                in_ds, DEVICE_PORT_TYPE_RECEIVE, port);

        if ((out != NULL) && (in != NULL))
            Audio_buffer_mix(out, in, buf_start, buf_stop);
    }

    return;
}


static void Au_state_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Au_state* au_state = (Au_state*)dstate;

    const Audio_unit* au = (const Audio_unit*)dstate->device;
    const Connections* connections = Audio_unit_get_connections(au);

    if (au_state->bypass)
    {
        mix_interface_connection(dstate, dstate, buf_start, buf_stop);
    }
    else if (connections != NULL)
    {
        //Connections_clear_buffers(au->connections, dstates, buf_start, buf_stop);

        Device_states* dstates = au_state->dstates;
        assert(dstates != NULL);

        // Fill input interface buffers
        Device_state* in_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Audio_unit_get_input_interface(au)));
        mix_interface_connection(in_iface_ds, dstate, buf_start, buf_stop);

        // Process audio unit graph
        Connections_mix(
                connections,
                au_state->dstates,
                wbs,
                buf_start,
                buf_stop,
                ((Device_state*)au_state)->audio_rate,
                tempo);

        // Fill output interface buffers
        Device_state* out_iface_ds = Device_states_get_state(
                dstates, Device_get_id(Audio_unit_get_output_interface(au)));
        mix_interface_connection(dstate, out_iface_ds, buf_start, buf_stop);
    }

    return;
}


Device_state* new_Au_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Au_state* au_state = memory_alloc_item(Au_state);
    if (au_state == NULL)
        return NULL;

    Au_state_init(au_state, device, audio_rate, audio_buffer_size);

    au_state->parent.render_mixed = Au_state_render_mixed;

    return &au_state->parent;
}


void Au_state_set_device_states(Au_state* au_state, Device_states* dstates)
{
    assert(au_state != NULL);
    assert(dstates != NULL);

    au_state->dstates = dstates;

    return;
}


void Au_state_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Au_state* au_state = (Au_state*)dstate;
    au_state->bypass = false;
    au_state->sustain = 0.0;

    return;
}


