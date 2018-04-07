

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_looper.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Looper_state.h>

#include <stdbool.h>
#include <stdlib.h>


static Set_float_func   Proc_looper_set_max_rec_time;
static Set_float_func   Proc_looper_set_state_xfade_time;
static Set_float_func   Proc_looper_set_play_xfade_time;

static void del_Proc_looper(Device_impl* dimpl);


Device_impl* new_Proc_looper(void)
{
    Proc_looper* looper = memory_alloc_item(Proc_looper);
    if (looper == NULL)
        return NULL;

    if (!Device_impl_init(&looper->parent, del_Proc_looper))
    {
        del_Device_impl(&looper->parent);
        return NULL;
    }

    looper->parent.create_pstate = new_Looper_pstate;

    looper->max_rec_time = LOOPER_DEFAULT_MAX_REC_TIME;

    looper->state_xfade_time = LOOPER_DEFAULT_STATE_XFADE_TIME;
    looper->play_xfade_time = LOOPER_DEFAULT_PLAY_XFADE_TIME;

    if (!(REGISTER_SET_WITH_STATE_CB(
                looper,
                float,
                max_rec_time,
                "p_f_max_rec_time.json",
                LOOPER_DEFAULT_MAX_REC_TIME,
                Looper_pstate_set_max_rec_time) &&
            REGISTER_SET_FIXED_STATE(
                looper,
                float,
                state_xfade_time,
                "p_f_state_xfade_time.json",
                LOOPER_DEFAULT_STATE_XFADE_TIME) &&
            REGISTER_SET_FIXED_STATE(
                looper,
                float,
                play_xfade_time,
                "p_f_play_xfade_time.json",
                LOOPER_DEFAULT_PLAY_XFADE_TIME)
        ))
    {
        del_Device_impl(&looper->parent);
        return NULL;
    }

    return &looper->parent;
}


static bool Proc_looper_set_max_rec_time(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_looper* looper = (Proc_looper*)dimpl;
    if (LOOPER_MIN_MAX_REC_TIME <= value && value <= LOOPER_MAX_MAX_REC_TIME)
        looper->max_rec_time = value;
    else
        looper->max_rec_time = LOOPER_DEFAULT_MAX_REC_TIME;

    return true;
}


static bool Proc_looper_set_state_xfade_time(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_looper* looper = (Proc_looper*)dimpl;
    looper->state_xfade_time = (value >= 0) ? value : LOOPER_DEFAULT_STATE_XFADE_TIME;

    return true;
}


static bool Proc_looper_set_play_xfade_time(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_looper* looper = (Proc_looper*)dimpl;
    looper->play_xfade_time = (value >= 0) ? value : LOOPER_DEFAULT_PLAY_XFADE_TIME;

    return true;
}


static void del_Proc_looper(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_looper* looper = (Proc_looper*)dimpl;
    memory_free(looper);

    return;
}


