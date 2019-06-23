

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_rangemap.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Rangemap_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static Set_float_func   Proc_rangemap_set_from_min;
static Set_float_func   Proc_rangemap_set_from_max;
static Set_float_func   Proc_rangemap_set_min_to;
static Set_float_func   Proc_rangemap_set_max_to;
static Set_bool_func    Proc_rangemap_set_clamp_dest_min;
static Set_bool_func    Proc_rangemap_set_clamp_dest_max;

static Device_impl_destroy_func del_Proc_rangemap;


Device_impl* new_Proc_rangemap(void)
{
    Proc_rangemap* rangemap = memory_alloc_item(Proc_rangemap);
    if (rangemap == NULL)
        return NULL;

    rangemap->from_min = 0.0;
    rangemap->from_max = 1.0;
    rangemap->min_to = 0.0;
    rangemap->max_to = 1.0;
    rangemap->clamp_dest_min = true;
    rangemap->clamp_dest_max = true;

    if (!Device_impl_init(&rangemap->parent, del_Proc_rangemap))
    {
        del_Device_impl(&rangemap->parent);
        return NULL;
    }

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(rangemap, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(rangemap, bool, name, keyp, def_value)

    if (!(REG_KEY(float, from_min, "p_f_from_min.json", 0.0) &&
            REG_KEY(float, from_max, "p_f_from_max.json", 1.0) &&
            REG_KEY(float, min_to, "p_f_min_to.json", 0.0) &&
            REG_KEY(float, max_to, "p_f_max_to.json", 1.0) &&
            REG_KEY_BOOL(clamp_dest_min, "p_b_clamp_dest_min.json", true) &&
            REG_KEY_BOOL(clamp_dest_max, "p_b_clamp_dest_max.json", true)
         ))
    {
        del_Device_impl(&rangemap->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    rangemap->parent.create_pstate = new_Rangemap_pstate;
    rangemap->parent.get_vstate_size = Rangemap_vstate_get_size;
    rangemap->parent.render_voice = Rangemap_vstate_render_voice;

    return &rangemap->parent;
}


static bool Proc_rangemap_set_from_min(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->from_min = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_rangemap_set_from_max(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->from_max = isfinite(value) ? value : 1.0;

    return true;
}


static bool Proc_rangemap_set_min_to(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->min_to = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_rangemap_set_max_to(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->max_to = isfinite(value) ? value : 1.0;

    return true;
}


static bool Proc_rangemap_set_clamp_dest_min(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->clamp_dest_min = enabled;

    return true;
}


static bool Proc_rangemap_set_clamp_dest_max(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->clamp_dest_max = enabled;

    return true;
}


static void del_Proc_rangemap(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    memory_free(rangemap);

    return;
}


