

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


#include <init/devices/processors/Proc_force.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Force_state.h>

#include <stdlib.h>


static Set_float_func       Proc_force_set_global_force;
static Set_float_func       Proc_force_set_force_variation;

static Set_envelope_func    Proc_force_set_env;
static Set_bool_func        Proc_force_set_env_enabled;
static Set_bool_func        Proc_force_set_env_loop_enabled;
static Set_float_func       Proc_force_set_env_scale_amount;
static Set_float_func       Proc_force_set_env_scale_center;


static void del_Proc_force(Device_impl* dimpl);


Device_impl* new_Proc_force(void)
{
    Proc_force* force = memory_alloc_item(Proc_force);
    if (force == NULL)
        return NULL;

    if (!Device_impl_init(&force->parent, del_Proc_force))
    {
        del_Device_impl(&force->parent);
        return NULL;
    }

    force->parent.get_vstate_size = Force_vstate_get_size;
    force->parent.init_vstate = Force_vstate_init;

    force->global_force = 0.0;
    force->force_var = 0.0;

    force->force_env = NULL;
    force->is_force_env_enabled = false;
    force->is_force_env_loop_enabled = false;
    force->force_env_scale_amount = 0.0;
    force->force_env_scale_center = 0.0;

    // Register key handlers
    bool reg_success = true;

#define REGISTER_KEY(type, field, key, def_val)                           \
    reg_success &= Device_impl_register_set_ ## type(                     \
            &force->parent, key, def_val, Proc_force_set_ ## field, NULL)

    REGISTER_KEY(float,     global_force,       "p_f_global_force.json",        0.0);
    REGISTER_KEY(float,     force_variation,    "p_f_force_variation.json",     0.0);
    REGISTER_KEY(envelope,  env,                "p_e_env.json",                 NULL);
    REGISTER_KEY(bool,      env_enabled,        "p_b_env_enabled.json",         false);
    REGISTER_KEY(bool,      env_loop_enabled,   "p_b_env_loop_enabled.json",    false);
    REGISTER_KEY(float,     env_scale_amount,   "p_f_env_scale_amount.json",    0.0);
    REGISTER_KEY(float,     env_scale_center,   "p_f_env_scale_center.json",    0.0);

#undef REGISTER_KEY

    return &force->parent;
}


static bool Proc_force_set_global_force(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->global_force = isfinite(value) ? value : 0.0;

    return true;
}


static bool Proc_force_set_force_variation(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_var = (isfinite(value) && (value >= 0)) ? value : 0.0;

    return true;
}


static bool Proc_force_set_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;

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

    force->force_env = is_valid ? value : NULL;

    return true;
}


static bool Proc_force_set_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_force_env_enabled = value;

    return true;
}


static bool Proc_force_set_env_loop_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->is_force_env_loop_enabled = value;

    return true;
}


static bool Proc_force_set_env_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_force_set_env_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_force* force = (Proc_force*)dimpl;
    force->force_env_scale_center = isfinite(value) ? value : 0;

    return true;
}


static void del_Proc_force(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_force* force = (Proc_force*)dimpl;
    memory_free(force);

    return;
}


