

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>

#include <Event_ins.h>


void Event_ins_process(Event_ins* event)
{
    assert(event != NULL);
    assert(EVENT_IS_INS(event->parent.type));
    assert(event->process != NULL);
    assert(event->ins_params != NULL);
    event->process(event);
    return;
}


void Event_ins_set_params(Event_ins* event, Instrument_params* ins_params)
{
    assert(event != NULL);
    assert(EVENT_IS_INS(event->parent.type));
    assert(ins_params != NULL);
    event->ins_params = ins_params;
    return;
}


