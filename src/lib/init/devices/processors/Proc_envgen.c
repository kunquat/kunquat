

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_envgen.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/param_types/Envelope.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Envgen_state.h>
#include <string/common.h>

#include <stdlib.h>
#include <string.h>


static Set_bool_func        Proc_envgen_set_time_env_enabled;
static Set_envelope_func    Proc_envgen_set_time_env;
static Set_bool_func        Proc_envgen_set_loop_enabled;
static Set_bool_func        Proc_envgen_set_release_env;
static Set_bool_func        Proc_envgen_set_linear_force;
static Set_float_func       Proc_envgen_set_global_adjust;
static Set_num_list_func    Proc_envgen_set_y_range;

static void del_Proc_envgen(Device_impl* dimpl);


Device_impl* new_Proc_envgen(void)
{
    Proc_envgen* envgen = memory_alloc_item(Proc_envgen);
    if (envgen == NULL)
        return NULL;

    envgen->is_time_env_enabled = false;
    envgen->time_env = NULL;
    envgen->is_loop_enabled = false;
    envgen->is_release_env = false;

    envgen->is_linear_force = false;

    envgen->global_adjust = 0;

    envgen->y_min = 0;
    envgen->y_max = 1;

    if (!Device_impl_init(&envgen->parent, del_Proc_envgen))
    {
        del_Device_impl(&envgen->parent);
        return NULL;
    }

    envgen->parent.get_vstate_size = Envgen_vstate_get_size;
    envgen->parent.init_vstate = Envgen_vstate_init;

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(envgen, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(envgen, bool, name, keyp, def_value)

    if (!(REG_KEY_BOOL(time_env_enabled, "p_b_env_enabled.json", false) &&
            REG_KEY(envelope, time_env, "p_e_env.json", NULL) &&
            REG_KEY_BOOL(loop_enabled, "p_b_env_loop_enabled.json", false) &&
            REG_KEY_BOOL(release_env, "p_b_env_is_release.json", false) &&
            REG_KEY_BOOL(linear_force, "p_b_linear_force.json", false) &&
            REG_KEY(float, global_adjust, "p_f_global_adjust.json", 0.0) &&
            REG_KEY(num_list, y_range, "p_ln_y_range.json", NULL)
        ))
    {
        del_Device_impl(&envgen->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    return &envgen->parent;
}


static bool Proc_envgen_set_time_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_time_env_enabled = value;

    return true;
}


static bool Proc_envgen_set_time_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = true;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        // Check the first node x coordinate
        {
            const double* node = Envelope_get_node(value, 0);
            if (node[0] != 0)
                is_valid = false;
        }

        // Check y coordinates
        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            const double* node = Envelope_get_node(value, i);
            if ((node[1] < 0) || (node[1] > 1))
            {
                is_valid = false;
                break;
            }
        }
    }
    else
    {
        is_valid = false;
    }

    egen->time_env = is_valid ? value : NULL;

    return true;
}


static bool Proc_envgen_set_loop_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_loop_enabled = value;

    return true;
}


static bool Proc_envgen_set_release_env(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_release_env = value;

    return true;
}


static bool Proc_envgen_set_linear_force(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    egen->is_linear_force = value;

    return true;
}


static bool Proc_envgen_set_global_adjust(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->global_adjust = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_envgen_set_y_range(
        Device_impl* dimpl, const Key_indices indices, const Num_list* value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = false;

    if ((value != NULL) && (Num_list_length(value) == 2))
    {
        const double n1 = Num_list_get_num(value, 0);
        const double n2 = Num_list_get_num(value, 1);

        if (isfinite(n1) && isfinite(n2) && (n1 <= n2))
            is_valid = true;
    }

    if (is_valid)
    {
        egen->y_min = Num_list_get_num(value, 0);
        egen->y_max = Num_list_get_num(value, 1);
    }
    else
    {
        egen->y_min = 0;
        egen->y_max = 1;
    }

    return true;
}


static void del_Proc_envgen(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    memory_free(egen);

    return;
}


