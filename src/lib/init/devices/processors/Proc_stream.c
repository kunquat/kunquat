

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
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
#include <init/devices/Proc_cons.h>
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
    stream->parent.render_voice = Stream_vstate_render_voice;

    // Register key handlers

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_WITH_STATE_CB(              \
            stream, type, name, keyp, def_value, Stream_pstate_set_ ## name)

    if (!(REG_KEY(float, init_value, "p_f_init_value.json", 0.0) &&
            REG_KEY(float, init_osc_speed, "p_f_init_osc_speed.json", 0.0) &&
            REG_KEY(float, init_osc_depth, "p_f_init_osc_depth.json", 0.0)
        ))
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

#undef REG_KEY

    return &stream->parent;
}


static bool Proc_stream_set_init_value(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->init_value = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_stream_set_init_osc_speed(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->init_osc_speed = (isfinite(value) && (value >= 0)) ? value : 0.0;

    return true;
}


static bool Proc_stream_set_init_osc_depth(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

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


