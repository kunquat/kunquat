

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
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
#include <memory.h>
#include <player/devices/processors/Filter_state.h>

#include <stdbool.h>


#define DEFAULT_CUTOFF 100.0
#define DEFAULT_RESONANCE 0.0


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

    filter->cutoff = DEFAULT_CUTOFF;
    filter->resonance = DEFAULT_RESONANCE;

    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &filter->parent,
            "p_f_cutoff.json",
            DEFAULT_CUTOFF,
            Proc_filter_set_cutoff,
            Filter_pstate_set_cutoff);

    reg_success &= Device_impl_register_set_float(
            &filter->parent,
            "p_f_resonance.json",
            DEFAULT_RESONANCE,
            Proc_filter_set_resonance,
            Filter_pstate_set_resonance);

    if (!reg_success)
    {
        del_Device_impl(&filter->parent);
        return NULL;
    }

    return &filter->parent;
}


static bool Proc_filter_set_cutoff(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

    Proc_filter* filter = (Proc_filter*)dimpl;
    filter->cutoff = value;

    return true;
}


static bool Proc_filter_set_resonance(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    assert(isfinite(value));

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


