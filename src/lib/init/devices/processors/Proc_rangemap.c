

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2020
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
#include <init/devices/param_types/Envelope.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Rangemap_state.h>

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static Set_float_func       Proc_rangemap_set_from_min;
static Set_float_func       Proc_rangemap_set_from_max;
static Set_float_func       Proc_rangemap_set_min_to;
static Set_float_func       Proc_rangemap_set_max_to;
static Set_bool_func        Proc_rangemap_set_clamp_dest_min;
static Set_bool_func        Proc_rangemap_set_clamp_dest_max;

static Set_bool_func        Proc_rangemap_set_env_enabled;
static Set_envelope_func    Proc_rangemap_set_envelope;

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

    rangemap->is_env_enabled = false;
    rangemap->envelope_param = NULL;
    rangemap->envelope = NULL;

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
            REG_KEY_BOOL(clamp_dest_max, "p_b_clamp_dest_max.json", true) &&
            REG_KEY_BOOL(env_enabled, "p_b_env_enabled.json", false) &&
            REG_KEY(envelope, envelope, "p_e_envelope.json", NULL)
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


static bool Proc_rangemap_update_envelope(Proc_rangemap* rangemap)
{
    rassert(rangemap != NULL);

    if (rangemap->envelope_param == NULL)
    {
        del_Envelope(rangemap->envelope);
        rangemap->envelope = NULL;
        return true;
    }

    const int old_node_count =
        (rangemap->envelope != NULL) ? Envelope_node_count(rangemap->envelope) : 0;
    const int new_node_count = Envelope_node_count(rangemap->envelope_param);

    if (old_node_count != new_node_count)
    {
        del_Envelope(rangemap->envelope);
        rangemap->envelope = NULL;

        rangemap->envelope = new_Envelope(
                new_node_count, -INFINITY, INFINITY, 0, -INFINITY, INFINITY, 0);
        if (rangemap->envelope == NULL)
            return false;

        for (int i = 0; i < new_node_count; ++i)
            Envelope_set_node(rangemap->envelope, i, 0);
    }

    for (int i = 0; i < new_node_count; ++i)
    {
        const double* param_node = Envelope_get_node(rangemap->envelope_param, i);
        double* node = Envelope_get_node(rangemap->envelope, i);

        node[0] = lerp(rangemap->from_min, rangemap->from_max, param_node[0]);
        node[1] = lerp(rangemap->min_to, rangemap->max_to, param_node[1]);
    }

    // Eliminate nodes with the same x coordinate
    double prev_x = -INFINITY;
    double seq_end_x = NAN;
    double seq_end_y = NAN;
    int write_pos = 0;
    for (int read_pos = 0; read_pos < new_node_count; ++read_pos)
    {
        double* node = Envelope_get_node(rangemap->envelope, write_pos);

        if (write_pos < read_pos)
        {
            double* src_node = Envelope_get_node(rangemap->envelope, read_pos);
            node[0] = src_node[0];
            node[1] = src_node[1];
        }

        if (node[0] == prev_x)
        {
            seq_end_x = nextafter(node[0], DBL_MAX);
            seq_end_y = node[1];
        }
        else
        {
            if (!isnan(seq_end_x))
            {
                if (node[0] > seq_end_x)
                {
                    rassert(write_pos < read_pos);
                    double* next_node =
                        Envelope_get_node(rangemap->envelope, write_pos + 1);
                    next_node[0] = node[0];
                    next_node[1] = node[1];
                    node[0] = seq_end_x;
                    node[1] = seq_end_y;

                    ++write_pos;
                }

                seq_end_x = NAN;
                seq_end_y = NAN;
            }

            ++write_pos;
        }

        prev_x = node[0];
    }

    const int final_node_count = write_pos;
    rassert(final_node_count >= 2);
    while (Envelope_node_count(rangemap->envelope) > final_node_count)
        Envelope_del_node(rangemap->envelope, final_node_count);

    return true;
}


static bool Proc_rangemap_set_from_min(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->from_min = isfinite(value) ? value : 0.0;

    if (!Proc_rangemap_update_envelope(rangemap))
        return false;

    return true;
}


static bool Proc_rangemap_set_from_max(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->from_max = isfinite(value) ? value : 1.0;

    if (!Proc_rangemap_update_envelope(rangemap))
        return false;

    return true;
}


static bool Proc_rangemap_set_min_to(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->min_to = isfinite(value) ? value : 0.0;

    if (!Proc_rangemap_update_envelope(rangemap))
        return false;

    return true;
}


static bool Proc_rangemap_set_max_to(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->max_to = isfinite(value) ? value : 1.0;

    if (!Proc_rangemap_update_envelope(rangemap))
        return false;

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


static bool Proc_rangemap_set_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    rangemap->is_env_enabled = enabled;

    return true;
}


static bool Proc_rangemap_set_envelope(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;

    bool is_valid = true;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        // Check x bounds
        {
            const double* first_node = Envelope_get_node(value, 0);
            if (first_node[0] != 0)
                is_valid = false;

            const double* last_node =
                Envelope_get_node(value, Envelope_node_count(value) - 1);
            if (last_node[0] != 1)
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

    if (is_valid)
    {
        rangemap->envelope_param = value;
        if (!Proc_rangemap_update_envelope(rangemap))
            return false;
    }
    else
    {
        rangemap->envelope_param = NULL;
        del_Envelope(rangemap->envelope);
        rangemap->envelope = NULL;
    }

    return true;
}


static void del_Proc_rangemap(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_rangemap* rangemap = (Proc_rangemap*)dimpl;
    del_Envelope(rangemap->envelope);
    rangemap->envelope = NULL;
    memory_free(rangemap);

    return;
}


