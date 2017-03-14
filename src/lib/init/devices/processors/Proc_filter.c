

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


#include <init/devices/processors/Proc_filter.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Filter_state.h>

#include <stdbool.h>


static Set_int_func   Proc_filter_set_type;
static Set_float_func Proc_filter_set_cutoff;
static Set_float_func Proc_filter_set_resonance;

static void del_Proc_filter(Device_impl* dimpl);


Device_impl* new_Proc_filter(void)
{
    Proc_filter* filter = memory_alloc_item(Proc_filter);
    if (filter == NULL)
        return NULL;

    if (!Device_impl_init(&filter->parent, del_Proc_filter))
    {
        del_Device_impl(&filter->parent);
        return NULL;
    }

    filter->parent.create_pstate = new_Filter_pstate;
    filter->parent.get_vstate_size = Filter_vstate_get_size;
    filter->parent.init_vstate = Filter_vstate_init;

    filter->type = FILTER_TYPE_LOWPASS;
    filter->cutoff = FILTER_DEFAULT_CUTOFF;
    filter->resonance = FILTER_DEFAULT_RESONANCE;

    if (!(REGISTER_SET_WITH_STATE_CB(
                filter,
                int,
                type,
                "p_i_type.json",
                FILTER_TYPE_LOWPASS,
                Filter_pstate_set_type) &&
            REGISTER_SET_WITH_STATE_CB(
                filter,
                float,
                cutoff,
                "p_f_cutoff.json",
                FILTER_DEFAULT_CUTOFF,
                Filter_pstate_set_cutoff) &&
            REGISTER_SET_WITH_STATE_CB(
                filter,
                float,
                resonance,
                "p_f_resonance.json",
                FILTER_DEFAULT_RESONANCE,
                Filter_pstate_set_resonance)
        ))
    {
        del_Device_impl(&filter->parent);
        return NULL;
    }

    return &filter->parent;
}


static bool Proc_filter_set_type(
        Device_impl* dimpl, const Key_indices indices, int64_t value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_filter* filter = (Proc_filter*)dimpl;
    if (value >= FILTER_TYPE_LOWPASS && value < FILTER_TYPE_COUNT)
        filter->type = value;
    else
        filter->type = FILTER_TYPE_LOWPASS;

    return true;
}


static bool Proc_filter_set_cutoff(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_filter* filter = (Proc_filter*)dimpl;
    filter->cutoff = value;

    return true;
}


static bool Proc_filter_set_resonance(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);
    rassert(isfinite(value));

    Proc_filter* filter = (Proc_filter*)dimpl;
    filter->resonance = value;

    return true;
}


static void del_Proc_filter(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_filter* filter = (Proc_filter*)dimpl;
    memory_free(filter);

    return;
}


