

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <devices/processors/Proc_delay.h>

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/processors/Delay_state.h>
#include <string/common.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DELAY_MAX_BUF_TIME 60

#define DELAY_DB_MAX 18


static bool Proc_delay_init(Device_impl* dimpl);

static Set_float_func Proc_delay_set_max_delay;
static Set_float_func Proc_delay_set_tap_delay;
static Set_float_func Proc_delay_set_tap_volume;

static void del_Proc_delay(Device_impl* dimpl);


Device_impl* new_Proc_delay(Processor* proc)
{
    Proc_delay* delay = memory_alloc_item(Proc_delay);
    if (delay == NULL)
        return NULL;

    delay->parent.device = (Device*)proc;

    Device_impl_register_init(&delay->parent, Proc_delay_init);
    Device_impl_register_destroy(&delay->parent, del_Proc_delay);

    return &delay->parent;
}


static bool Proc_delay_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_delay* delay = (Proc_delay*)dimpl;

    Device_set_state_creator(delay->parent.device, new_Delay_pstate);

    // Register key handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &delay->parent,
            "p_f_max_delay.json",
            2.0,
            Proc_delay_set_max_delay,
            Delay_pstate_set_max_delay);
    reg_success &= Device_impl_register_set_float(
            &delay->parent,
            "tap_XX/p_f_delay.json",
            -1.0,
            Proc_delay_set_tap_delay,
            Delay_pstate_set_tap_delay);
    reg_success &= Device_impl_register_set_float(
            &delay->parent,
            "tap_XX/p_f_volume.json",
            0.0,
            Proc_delay_set_tap_volume,
            Delay_pstate_set_tap_volume);

    if (!reg_success)
        return false;

    // TODO: add control variable accessors

    delay->max_delay = 2;

    for (int i = 0; i < DELAY_TAPS_MAX; ++i)
    {
        delay->taps[i].delay = INFINITY;
        delay->taps[i].scale = 1;
    }

    return true;
}


static double get_tap_delay(double value)
{
    return (value >= 0) ? value : INFINITY;
}

static double get_tap_volume(double value)
{
    return (isfinite(value) && value < DELAY_DB_MAX) ? exp2(value / 6) : 1.0;
}


static bool Proc_delay_set_max_delay(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_delay* delay = (Proc_delay*)dimpl;
    delay->max_delay = min(value, DELAY_MAX_BUF_TIME);

    return true;
}


static bool Proc_delay_set_tap_delay(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= DELAY_TAPS_MAX)
        return true;

    Proc_delay* delay = (Proc_delay*)dimpl;
    delay->taps[indices[0]].delay = get_tap_delay(value);

    return true;
}


static bool Proc_delay_set_tap_volume(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= DELAY_TAPS_MAX)
        return true;

    Proc_delay* delay = (Proc_delay*)dimpl;
    delay->taps[indices[0]].scale = get_tap_volume(value);

    return true;
}


static void del_Proc_delay(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_delay* delay = (Proc_delay*)dimpl;
    memory_free(delay);

    return;
}


