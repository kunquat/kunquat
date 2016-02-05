

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_stream.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Stream_state.h>

#include <stdlib.h>


static Set_float_func   Proc_stream_set_init_value;
static Set_float_func   Proc_stream_set_init_osc_speed;
static Set_float_func   Proc_stream_set_init_osc_depth;


static void del_Proc_stream(Device_impl* dimpl);


Device_impl* new_Proc_stream(void)
{
    Proc_stream* stream = memory_alloc_item(Proc_stream);
    if (stream == NULL)
        return NULL;

    stream->init_value = 0;
    stream->init_osc_speed = 0;
    stream->init_osc_depth = 0;

    if (!Device_impl_init(&stream->parent, del_Proc_stream))
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

    stream->parent.create_pstate = new_Stream_pstate;
    stream->parent.get_vstate_size = Stream_vstate_get_size;
    stream->parent.init_vstate = Stream_vstate_init;

    // Register key handlers, TODO: clean this up
    if (!(REGISTER_SET_WITH_STATE_CB(
                stream, float, init_value, "p_f_init_value.json", 0.0, Stream_pstate_set_init_value) &&
            REGISTER_SET_WITH_STATE_CB(
                stream, float, init_osc_speed, "p_f_init_osc_speed.json", 0.0, Stream_pstate_set_init_osc_speed) &&
            REGISTER_SET_WITH_STATE_CB(
                stream, float, init_osc_depth, "p_f_init_osc_depth.json", 0.0, Stream_pstate_set_init_osc_depth)
        ))
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

    return &stream->parent;
}


static bool Proc_stream_set_init_value(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->init_value = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_stream_set_init_osc_speed(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->init_osc_speed = (isfinite(value) && (value >= 0)) ? value : 0.0;

    return true;
}


static bool Proc_stream_set_init_osc_depth(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->init_osc_depth = (isfinite(value) && (value >= 0)) ? value : 0.0;

    return true;
}


static void del_Proc_stream(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_stream* stream = (Proc_stream*)dimpl;
    memory_free(stream);

    return;
}


