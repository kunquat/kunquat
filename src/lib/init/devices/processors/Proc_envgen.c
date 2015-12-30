

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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
#include <init/devices/Processor.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Envgen_state.h>
#include <string/common.h>

#include <stdlib.h>
#include <string.h>


static Set_float_func       Proc_envgen_set_scale;
static Set_bool_func        Proc_envgen_set_time_env_enabled;
static Set_envelope_func    Proc_envgen_set_time_env;
static Set_bool_func        Proc_envgen_set_loop_enabled;
static Set_float_func       Proc_envgen_set_env_scale_amount;
static Set_float_func       Proc_envgen_set_env_scale_center;
static Set_bool_func        Proc_envgen_set_force_env_enabled;
static Set_envelope_func    Proc_envgen_set_force_env;
static Set_num_list_func    Proc_envgen_set_y_range;

static void del_Proc_envgen(Device_impl* dimpl);


Device_impl* new_Proc_envgen(void)
{
    Proc_envgen* egen = memory_alloc_item(Proc_envgen);
    if (egen == NULL)
        return NULL;

    if (!Device_impl_init(&egen->parent, del_Proc_envgen))
    {
        del_Device_impl(&egen->parent);
        return NULL;
    }

    egen->parent.init_vstate = Envgen_vstate_init;

    egen->scale = 1;

    egen->is_time_env_enabled = false;
    egen->time_env = NULL;
    egen->is_loop_enabled = false;
    egen->env_scale_amount = 0;
    egen->env_scale_center = 440;

    egen->is_force_env_enabled = false;
    egen->force_env = NULL;

    egen->y_min = 0;
    egen->y_max = 1;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val)                         \
    reg_success &= Device_impl_register_set_##type(                     \
            &egen->parent, key, def_val, Proc_envgen_set_##field, NULL)

    REGISTER_SET(float,     scale,              "p_f_scale.json",               0.0);
    REGISTER_SET(bool,      time_env_enabled,   "p_b_env_enabled.json",         false);
    REGISTER_SET(envelope,  time_env,           "p_e_env.json",                 NULL);
    REGISTER_SET(bool,      loop_enabled,       "p_b_env_loop_enabled.json",    false);
    REGISTER_SET(float,     env_scale_amount,   "p_f_env_scale_amount.json",    0.0);
    REGISTER_SET(float,     env_scale_center,   "p_f_env_scale_center.json",    0.0);
    REGISTER_SET(bool,      force_env_enabled,  "p_b_force_env_enabled.json",   false);
    REGISTER_SET(envelope,  force_env,          "p_e_force_env.json",           NULL);
    REGISTER_SET(num_list,  y_range,            "p_ln_y_range.json",            NULL);

#undef REGISTER_SET

    if (!reg_success)
    {
        del_Device_impl(&egen->parent);
        return NULL;
    }

    return &egen->parent;
}


const char* Proc_envgen_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", Envgen_vstate_get_size());

        return size_str;
    }

    return NULL;
}


static bool Proc_envgen_set_scale(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->scale = isfinite(value) ? exp2(value / 6) : 1;

    return true;
}


static bool Proc_envgen_set_time_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_time_env_enabled = value;

    return true;
}


static bool Proc_envgen_set_time_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

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
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_loop_enabled = value;

    return true;
}


static bool Proc_envgen_set_env_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_envgen_set_env_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->env_scale_center = isfinite(value) ? exp2(value / 1200) * 440 : 440;

    return true;
}


static bool Proc_envgen_set_force_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_force_env_enabled = value;

    return true;
}


static bool Proc_envgen_set_force_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = false;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        // Check the endpoint x coordinates
        const double* first_node = Envelope_get_node(value, 0);
        const double* last_node = Envelope_get_node(
                value, Envelope_node_count(value) - 1);

        if ((first_node[0] == 0) && (last_node[0] == 1))
        {
            // Check y coordinates
            bool invalid_found = false;

            for (int i = 0; i < Envelope_node_count(value); ++i)
            {
                const double* node = Envelope_get_node(value, i);
                if ((node[1] < 0) || (node[1] > 1))
                {
                    invalid_found = true;
                    break;
                }
            }

            if (!invalid_found)
                is_valid = true;
        }
    }

    egen->force_env = is_valid ? value : NULL;

    return true;
}


static bool Proc_envgen_set_y_range(
        Device_impl* dimpl, const Key_indices indices, const Num_list* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

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


