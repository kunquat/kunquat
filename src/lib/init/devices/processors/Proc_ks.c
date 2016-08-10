

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


#include <init/devices/processors/Proc_ks.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Ks_state.h>

#include <stdlib.h>
#include <string.h>


#define DEFAULT_SHIFT_ENV_TRIG_THRESHOLD 40


static Set_float_func       Proc_ks_set_damp;

static Set_envelope_func    Proc_ks_set_init_env;
static Set_bool_func        Proc_ks_set_init_env_loop_enabled;
static Set_float_func       Proc_ks_set_init_env_scale_amount;
static Set_float_func       Proc_ks_set_init_env_scale_center;

static Set_envelope_func    Proc_ks_set_shift_env;
static Set_bool_func        Proc_ks_set_shift_env_enabled;
static Set_float_func       Proc_ks_set_shift_env_scale_amount;
static Set_float_func       Proc_ks_set_shift_env_scale_center;
static Set_float_func       Proc_ks_set_shift_env_trig_threshold;
static Set_float_func       Proc_ks_set_shift_env_trig_strength_var;

static Device_impl_get_voice_wb_size_func Proc_ks_get_voice_wb_size;

static Device_impl_destroy_func del_Proc_ks;


static Envelope* make_default_envelope(const char* env_data)
{
    Envelope* env = new_Envelope(2, 0, 1, 0, 0, 1, 0);
    if (env == NULL)
        return NULL;

    Streader* sr = Streader_init(STREADER_AUTO, env_data, (int64_t)strlen(env_data));

    if (!Envelope_read(env, sr))
    {
        // The default envelope should be valid
        rassert((sr->error.type == ERROR_MEMORY) || (sr->error.type == ERROR_RESOURCE));

        del_Envelope(env);
        return NULL;
    }

    return env;
}


Device_impl* new_Proc_ks(void)
{
    Proc_ks* ks = memory_alloc_item(Proc_ks);
    if (ks == NULL)
        return NULL;

    ks->damp = 0;

    ks->init_env = NULL;
    ks->is_init_env_loop_enabled = false;
    ks->init_env_scale_amount = 0;
    ks->init_env_scale_center = 0;
    ks->def_init_env = NULL;

    ks->shift_env = NULL;
    ks->is_shift_env_enabled = false;
    ks->shift_env_scale_amount = 0;
    ks->shift_env_scale_center = 0;
    ks->def_shift_env = NULL;
    ks->shift_env_trig_threshold = DEFAULT_SHIFT_ENV_TRIG_THRESHOLD;
    ks->shift_env_trig_strength_var = 0;

    if (!Device_impl_init(&ks->parent, del_Proc_ks))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

    // Add default envelopes
    {
        static const char* init_env_data =
            "{ \"nodes\": [ [0, 1], [0.01, 0] ], \"marks\": [0, 1] }";
        ks->def_init_env = make_default_envelope(init_env_data);
        if (ks->def_init_env == NULL)
        {
            del_Device_impl(&ks->parent);
            return NULL;
        }

        ks->init_env = ks->def_init_env;

        static const char* shift_env_data =
            "{ \"nodes\": [ [0, 1], [0.001, 0] ] }";
        ks->def_shift_env = make_default_envelope(shift_env_data);
        if (ks->def_shift_env == NULL)
        {
            del_Device_impl(&ks->parent);
            return NULL;
        }

        ks->shift_env = ks->def_shift_env;
    }

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(ks, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(ks, bool, name, keyp, def_value)

    if (!(REG_KEY(float, damp, "p_f_damp.json", 0.0) &&
                REG_KEY(envelope, init_env, "p_e_init_env.json", NULL) &&
                REG_KEY_BOOL(init_env_loop_enabled,
                    "p_b_init_env_loop_enabled.json", false) &&
                REG_KEY(float, init_env_scale_amount,
                    "p_f_init_env_scale_amount.json", 0.0) &&
                REG_KEY(float, init_env_scale_center,
                    "p_f_init_env_scale_center.json", 0.0) &&
                REG_KEY(envelope, shift_env, "p_e_shift_env.json", NULL) &&
                REG_KEY_BOOL(shift_env_enabled, "p_b_shift_env_enabled.json", false) &&
                REG_KEY(float, shift_env_scale_amount,
                    "p_f_shift_env_scale_amount.json", 0.0) &&
                REG_KEY(float, shift_env_scale_center,
                    "p_f_shift_env_scale_center.json", 0.0) &&
                REG_KEY(float, shift_env_trig_threshold,
                    "p_f_shift_env_trig_threshold.json",
                    DEFAULT_SHIFT_ENV_TRIG_THRESHOLD) &&
                REG_KEY(float, shift_env_trig_strength_var,
                    "p_f_shift_env_trig_strength_var.json", 0.0)
         ))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    ks->parent.get_voice_wb_size = Proc_ks_get_voice_wb_size;
    ks->parent.get_vstate_size = Ks_vstate_get_size;
    ks->parent.init_vstate = Ks_vstate_init;

    return &ks->parent;
}


static bool Proc_ks_set_damp(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;

    if (value >= 0 && value <= 100)
        ks->damp = value;
    else
        ks->damp = 0;

    return true;
}


static bool is_valid_excit_envelope(const Envelope* env)
{
    if (env == NULL)
        return false;

    const int node_count = Envelope_node_count(env);
    if (node_count < 2 || node_count > 32)
        return false;

    // Check the first node x coordinate
    {
        const double* node = Envelope_get_node(env, 0);
        if (node[0] != 0)
            return false;
    }

    // Check y coordinates
    for (int i = 0; i < node_count; ++i)
    {
        const double* node = Envelope_get_node(env, i);
        if (node[1] < 0 || node[1] > 1)
            return false;
    }

    return true;
}


static bool Proc_ks_set_init_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->init_env = is_valid_excit_envelope(value) ? value : ks->def_init_env;

    return true;
}


static bool Proc_ks_set_init_env_loop_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->is_init_env_loop_enabled = value;

    return true;
}


static bool Proc_ks_set_init_env_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->init_env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_ks_set_init_env_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->init_env_scale_center = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_ks_set_shift_env(
        Device_impl* dimpl, const Key_indices indices, const Envelope* value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    bool is_valid = false;
    if (is_valid_excit_envelope(value))
    {
        const int node_count = Envelope_node_count(value);
        const double* last_node = Envelope_get_node(value, node_count - 1);
        if (last_node[1] == 0)
            is_valid = true;
    }

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->shift_env = is_valid ? value : ks->def_shift_env;

    return true;
}


static bool Proc_ks_set_shift_env_enabled(
        Device_impl* dimpl, const Key_indices indices, bool value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->is_shift_env_enabled = value;

    return true;
}


static bool Proc_ks_set_shift_env_scale_amount(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->shift_env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_ks_set_shift_env_scale_center(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->shift_env_scale_center = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_ks_set_shift_env_trig_threshold(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->shift_env_trig_threshold =
        (isfinite(value) && value > 0) ? value : DEFAULT_SHIFT_ENV_TRIG_THRESHOLD;

    return true;
}


static bool Proc_ks_set_shift_env_trig_strength_var(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;
    ks->shift_env_trig_strength_var = (isfinite(value) && value >= 0) ? value : 0;

    return true;
}


static int32_t Proc_ks_get_voice_wb_size(const Device_impl* dimpl, int32_t audio_rate)
{
    rassert(dimpl != NULL);
    rassert(audio_rate > 0);

    static const double min_freq = 10;

    return (int32_t)(audio_rate / min_freq) + 1;
}


static void del_Proc_ks(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ks* ks = (Proc_ks*)dimpl;
    del_Envelope(ks->def_init_env);
    memory_free(ks);

    return;
}


