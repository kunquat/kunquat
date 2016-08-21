

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


#include <init/devices/processors/Proc_compress.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Compress_state.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define DEFAULT_ATTACK 1.0
#define DEFAULT_RELEASE 100.0

#define DEFAULT_UPWARD_THRESHOLD -60.0
#define DEFAULT_UPWARD_RANGE 12.0
#define DEFAULT_DOWNWARD_THRESHOLD 0.0
#define DEFAULT_DOWNWARD_RANGE 24.0
#define DEFAULT_RATIO 6.0


static Set_float_func   Proc_compress_set_attack;
static Set_float_func   Proc_compress_set_release;

static Set_bool_func    Proc_compress_set_upward_enabled;
static Set_float_func   Proc_compress_set_upward_threshold;
static Set_float_func   Proc_compress_set_upward_range;
static Set_float_func   Proc_compress_set_upward_ratio;

static Set_bool_func    Proc_compress_set_downward_enabled;
static Set_float_func   Proc_compress_set_downward_threshold;
static Set_float_func   Proc_compress_set_downward_range;
static Set_float_func   Proc_compress_set_downward_ratio;

static Device_impl_destroy_func del_Proc_compress;


Device_impl* new_Proc_compress(void)
{
    Proc_compress* compress = memory_alloc_item(Proc_compress);
    if (compress == NULL)
        return NULL;

    compress->attack = DEFAULT_ATTACK;
    compress->release = DEFAULT_RELEASE;

    compress->upward_enabled = false;
    compress->upward_threshold = DEFAULT_UPWARD_THRESHOLD;
    compress->upward_range = DEFAULT_UPWARD_RANGE;
    compress->upward_ratio = DEFAULT_RATIO;

    compress->downward_enabled = false;
    compress->downward_threshold = DEFAULT_DOWNWARD_THRESHOLD;
    compress->downward_range = DEFAULT_DOWNWARD_RANGE;
    compress->downward_ratio = DEFAULT_RATIO;

    if (!Device_impl_init(&compress->parent, del_Proc_compress))
    {
        del_Device_impl(&compress->parent);
        return NULL;
    }

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(compress, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(compress, bool, name, keyp, def_value)

    if (!(REG_KEY(float, attack, "p_f_attack.json", DEFAULT_ATTACK) &&
            REG_KEY(float, release, "p_f_release.json", DEFAULT_RELEASE) &&
            REG_KEY_BOOL(upward_enabled, "p_b_upward_enabled.json", false) &&
            REG_KEY(float, upward_threshold,
                "p_f_upward_threshold.json", DEFAULT_UPWARD_THRESHOLD) &&
            REG_KEY(float, upward_range,
                "p_f_upward_range.json", DEFAULT_UPWARD_RANGE) &&
            REG_KEY(float, upward_ratio, "p_f_upward_ratio.json", DEFAULT_RATIO) &&
            REG_KEY_BOOL(downward_enabled, "p_b_downward_enabled.json", false) &&
            REG_KEY(float, downward_threshold,
                "p_f_downward_threshold.json", DEFAULT_DOWNWARD_THRESHOLD) &&
            REG_KEY(float, downward_range,
                "p_f_downward_range.json", DEFAULT_DOWNWARD_RANGE) &&
            REG_KEY(float, downward_ratio, "p_f_downward_ratio.json", DEFAULT_RATIO)
         ))
    {
        del_Device_impl(&compress->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    compress->parent.create_pstate = new_Compress_pstate;
    compress->parent.get_vstate_size = Compress_vstate_get_size;
    compress->parent.init_vstate = Compress_vstate_init;

    return &compress->parent;
}


static bool is_valid_reaction_time(double value)
{
    return isfinite(value) && (value >= 1.0) && (value <= 2000.0);
}


static bool Proc_compress_set_attack(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->attack = is_valid_reaction_time(value) ? value : DEFAULT_ATTACK;

    return true;
}


static bool Proc_compress_set_release(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->release = is_valid_reaction_time(value) ? value : DEFAULT_RELEASE;

    return true;
}


static bool Proc_compress_set_upward_enabled(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->upward_enabled = enabled;

    return true;
}


static bool is_valid_threshold(double value)
{
    if (!isfinite(value))
        return false;

    return (value >= DEFAULT_UPWARD_THRESHOLD) && (value <= DEFAULT_DOWNWARD_THRESHOLD);
}


static bool is_valid_range(double value)
{
    if (!isfinite(value))
        return false;

    return (value >= 0.0) && (value <= 60.0);
}


static bool is_valid_ratio(double value)
{
    if (!isfinite(value))
        return false;

    return (value >= 1.0) && (value <= 60.0);
}


static bool Proc_compress_set_upward_threshold(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->upward_threshold =
        is_valid_threshold(value) ? value : DEFAULT_UPWARD_THRESHOLD;

    return true;
}


static bool Proc_compress_set_upward_range(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->upward_range = is_valid_range(value) ? value : DEFAULT_UPWARD_RANGE;

    return true;
}


static bool Proc_compress_set_upward_ratio(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->upward_ratio = is_valid_ratio(value) ? value : DEFAULT_RATIO;

    return true;
}


static bool Proc_compress_set_downward_enabled(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->downward_enabled = enabled;

    return true;
}


static bool Proc_compress_set_downward_threshold(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->downward_threshold =
        is_valid_threshold(value) ? value : DEFAULT_DOWNWARD_THRESHOLD;

    return true;
}


static bool Proc_compress_set_downward_range(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->downward_range = is_valid_range(value) ? value : DEFAULT_DOWNWARD_RANGE;

    return true;
}


static bool Proc_compress_set_downward_ratio(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_compress* compress = (Proc_compress*)dimpl;
    compress->downward_ratio = is_valid_ratio(value) ? value : DEFAULT_RATIO;

    return true;
}


static void del_Proc_compress(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_compress* compress = (Proc_compress*)dimpl;
    memory_free(compress);

    return;
}


