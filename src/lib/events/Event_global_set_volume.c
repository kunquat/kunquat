

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_master_decl.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_set_volume_process(Playdata* global_state, Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_FLOAT)
    {
        return false;
    }
    global_state->volume = exp2(value->value.float_type / 6);
    Slider_break(&global_state->volume_slider);
//    global_state->volume_slide = 0;
    return true;
}


