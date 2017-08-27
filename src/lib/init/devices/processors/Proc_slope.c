

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_slope.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Slope_state.h>

#include <stdint.h>
#include <stdlib.h>


static Set_bool_func    Proc_slope_set_absolute;
static Set_float_func   Proc_slope_set_smoothing;

static Device_impl_destroy_func del_Proc_slope;


Device_impl* new_Proc_slope(void)
{
    Proc_slope* slope = memory_alloc_item(Proc_slope);
    if (slope == NULL)
        return NULL;

    slope->absolute = false;
    slope->smoothing = 0;

    if (!Device_impl_init(&slope->parent, del_Proc_slope))
    {
        del_Device_impl(&slope->parent);
        return NULL;
    }

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(slope, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(slope, bool, name, keyp, def_value)

    if (!(REG_KEY_BOOL(absolute, "p_b_absolute.json", false) &&
            REG_KEY(float, smoothing, "p_f_smoothing.json", 0.0)
        ))
    {
        del_Device_impl(&slope->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    slope->parent.create_pstate = new_Slope_pstate;
    slope->parent.get_vstate_size = Slope_vstate_get_size;
    slope->parent.init_vstate = Slope_vstate_init;
    slope->parent.render_voice = Slope_vstate_render_voice;

    return &slope->parent;
}


static bool Proc_slope_set_absolute(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_slope* slope = (Proc_slope*)dimpl;
    slope->absolute = value;

    return true;
}


static bool Proc_slope_set_smoothing(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_slope* slope = (Proc_slope*)dimpl;
    slope->smoothing = (value >= 0 && value <= 10) ? value : 0.0;

    return true;
}


static void del_Proc_slope(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_slope* slope = (Proc_slope*)dimpl;
    memory_free(slope);

    return;
}


