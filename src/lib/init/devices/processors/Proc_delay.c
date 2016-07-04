

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_delay.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/processors/Delay_state.h>
#include <string/common.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Set_float_func Proc_delay_set_max_delay;
static Set_float_func Proc_delay_set_init_delay;

static void del_Proc_delay(Device_impl* dimpl);


Device_impl* new_Proc_delay(void)
{
    Proc_delay* delay = memory_alloc_item(Proc_delay);
    if (delay == NULL)
        return NULL;

    delay->max_delay = DELAY_DEFAULT_BUF_LENGTH;
    delay->init_delay = 0;

    if (!Device_impl_init(&delay->parent, del_Proc_delay))
    {
        del_Device_impl(&delay->parent);
        return NULL;
    }

    delay->parent.create_pstate = new_Delay_pstate;

    // Register key set/update handlers
    if (!(REGISTER_SET_WITH_STATE_CB(
                delay,
                float,
                max_delay,
                "p_f_max_delay.json",
                delay->max_delay,
                Delay_pstate_set_max_delay) &&
            REGISTER_SET_FIXED_STATE(
                delay, float, init_delay, "p_f_init_delay.json", delay->init_delay)
         ))
    {
        del_Device_impl(&delay->parent);
        return NULL;
    }

    return &delay->parent;
}


static bool Proc_delay_set_max_delay(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    ignore(indices);

    Proc_delay* delay = (Proc_delay*)dimpl;

    if (isfinite(value) && (value > 0) && (value <= DELAY_MAX_BUF_LENGTH))
        delay->max_delay = value;
    else
        delay->max_delay = DELAY_DEFAULT_BUF_LENGTH;

    return true;
}


static bool Proc_delay_set_init_delay(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    ignore(indices);

    Proc_delay* delay = (Proc_delay*)dimpl;

    delay->init_delay = (isfinite(value) && (value >= 0)) ? value : 0;

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


