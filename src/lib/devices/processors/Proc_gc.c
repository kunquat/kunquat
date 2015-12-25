

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_gc.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/param_types/Envelope.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Voice_state.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Proc_gc
{
    Device_impl parent;

    bool is_map_enabled;
    const Envelope* map;
} Proc_gc;


static Set_bool_func     Proc_gc_set_map_enabled;
static Set_envelope_func Proc_gc_set_map;

static bool Proc_gc_init(Device_impl* dimpl);

static Voice_state_render_voice_func Gc_state_render_voice;

static void Gc_state_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);

static void del_Proc_gc(Device_impl* dimpl);


Device_impl* new_Proc_gc(Processor* proc)
{
    Proc_gc* gc = memory_alloc_item(Proc_gc);
    if (gc == NULL)
        return NULL;

    gc->parent.device = (Device*)proc;

    Device_impl_register_init(&gc->parent, Proc_gc_init);
    Device_impl_register_destroy(&gc->parent, del_Proc_gc);

    return &gc->parent;
}


static Device_state* Proc_gc_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_mixed = Gc_state_render_mixed;

    return (Device_state*)proc_state;
}


static void Proc_gc_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(vstate != NULL);

    vstate->render_voice = Gc_state_render_voice;

    return;
}


static bool Proc_gc_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_gc* gc = (Proc_gc*)dimpl;

    Device_set_state_creator(dimpl->device, Proc_gc_create_state);

    Processor* proc = (Processor*)gc->parent.device;
    proc->init_vstate = Proc_gc_init_vstate;

    gc->is_map_enabled = false;
    gc->map = NULL;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val)                   \
    reg_success &= Device_impl_register_set_##type(               \
            &gc->parent, key, def_val, Proc_gc_set_##field, NULL)

    REGISTER_SET(bool,      map_enabled,    "p_b_map_enabled.json",     false);
    REGISTER_SET(envelope,  map,            "p_e_map.json",             NULL);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    return true;
}


static bool Proc_gc_set_map_enabled(Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_gc* gc = (Proc_gc*)dimpl;
    gc->is_map_enabled = value;

    return true;
}


static bool Proc_gc_set_map(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_gc* gc = (Proc_gc*)dimpl;

    bool valid = true;
    if (value != NULL && Envelope_node_count(value) > 1)
    {
        double* node = Envelope_get_node(value, 0);
        if (node[0] != 0)
            valid = false;

        node = Envelope_get_node(value, Envelope_node_count(value) - 1);
        if (node[0] != 1)
            valid = false;

        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            node = Envelope_get_node(value, i);
            if (node[1] < 0)
            {
                valid = false;
                break;
            }
        }
    }
    else
    {
        valid = false;
    }

    gc->map = valid ? value : NULL;

    return true;
}


static void distort(
        const Proc_gc* gc,
        Audio_buffer* in_buffer,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(gc != NULL);
    assert(in_buffer != NULL);
    assert(out_buffer != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);

    if (gc->is_map_enabled && (gc->map != NULL))
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* in_values = Audio_buffer_get_buffer(in_buffer, ch);
            float* out_values = Audio_buffer_get_buffer(out_buffer, ch);

            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const float in_value = in_values[i];
                const float abs_value = fabs(in_value);

                float out_value = Envelope_get_value(gc->map, min(abs_value, 1));
                if (in_value < 0)
                    out_value = -out_value;

                out_values[i] = out_value;
            }
        }
    }
    else
    {
        Audio_buffer_copy(out_buffer, in_buffer, buf_start, buf_stop);
    }

    return;
}


static int32_t Gc_state_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get input
    Audio_buffer* in_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (in_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    // Distort the signal
    const Proc_gc* gc = (const Proc_gc*)proc_state->parent.device->dimpl;
    distort(gc, in_buffer, out_buffer, buf_start, buf_stop);

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


static void Gc_state_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(tempo > 0);

    // Get input
    Audio_buffer* in_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (in_buffer == NULL)
        return;

    // Get output
    Audio_buffer* out_buffer =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    // Distort the signal
    const Proc_gc* gc = (const Proc_gc*)dstate->device->dimpl;
    distort(gc, in_buffer, out_buffer, buf_start, buf_stop);

    return;
}


static void del_Proc_gc(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_gc* gc = (Proc_gc*)dimpl;
    memory_free(gc);

    return;
}


