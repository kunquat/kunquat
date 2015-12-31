

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


#include <init/devices/processors/Proc_chorus.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/processors/Chorus_state.h>
#include <string/common.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CHORUS_PARAM(name, dev_key, update_key, def_value) \
    static Set_float_func Proc_chorus_set_voice_ ## name;
#include <init/devices/processors/Proc_chorus_params.h>


static void del_Proc_chorus(Device_impl* dimpl);


Device_impl* new_Proc_chorus(void)
{
    Proc_chorus* chorus = memory_alloc_item(Proc_chorus);
    if (chorus == NULL)
        return NULL;

    if (!Device_impl_init(&chorus->parent, del_Proc_chorus))
    {
        del_Device_impl(&chorus->parent);
        return NULL;
    }

    chorus->parent.create_pstate = new_Chorus_pstate;

    // Register key set/update handlers
    bool reg_success = true;

#define CHORUS_PARAM(name, dev_key, update_key, def_value) \
    reg_success &= Device_impl_register_set_float(         \
            &chorus->parent,                               \
            dev_key,                                       \
            def_value,                                     \
            Proc_chorus_set_voice_ ## name,                \
            Chorus_pstate_set_voice_ ## name);
#include <init/devices/processors/Proc_chorus_params.h>

    // Control variables
    reg_success &= Device_impl_create_cv_float(
            &chorus->parent,
            "voice_XX/delay",
            Chorus_pstate_get_cv_delay_variance,
            NULL);

    reg_success &= Device_impl_create_cv_float(
            &chorus->parent, "voice_XX/volume", Chorus_pstate_get_cv_volume, NULL);

    if (!reg_success)
    {
        del_Device_impl(&chorus->parent);
        return NULL;
    }

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice_params* params = &chorus->voice_params[i];

        params->delay = -1;
        params->range = 0;
        params->speed = 0;
        params->volume = 1;
    }

    return &chorus->parent;
}


static double get_voice_delay(double value)
{
    return ((0 <= value) && (value < CHORUS_DELAY_MAX)) ? value : -1.0;
}


static double get_voice_range(double value)
{
    // The negation below flips the oscillation phase to make it more consistent
    return ((0 <= value) && (value < CHORUS_DELAY_MAX)) ? -value : 0.0;
}


static double get_voice_speed(double value)
{
    return (value >= 0) ? value : 0.0;
}


static double get_voice_volume(double value)
{
    return (value <= CHORUS_DB_MAX) ? value : 0.0;
}


#define CHORUS_PARAM(name, dev_key, update_key, def_value)               \
    static bool Proc_chorus_set_voice_ ## name(                          \
            Device_impl* dimpl, const Key_indices indices, double value) \
    {                                                                    \
        assert(dimpl != NULL);                                           \
        assert(indices != NULL);                                         \
                                                                         \
        if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)           \
            return true;                                                 \
                                                                         \
        Proc_chorus* chorus = (Proc_chorus*)dimpl;                       \
        Chorus_voice_params* params = &chorus->voice_params[indices[0]]; \
                                                                         \
        params->name = get_voice_ ## name(value);                        \
                                                                         \
        return true;                                                     \
    }
#include <init/devices/processors/Proc_chorus_params.h>


static void del_Proc_chorus(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_chorus* chorus = (Proc_chorus*)dimpl;
    memory_free(chorus);

    return;
}


