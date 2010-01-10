

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


#ifndef K_EVENT_INS_SET_PEDAL_H
#define K_EVENT_INS_SET_PEDAL_H


#include <Event_ins.h>
#include <Reltime.h>


typedef struct Event_ins_set_pedal
{
    Event_ins parent;
    double pedal;
} Event_ins_set_pedal;


Event* new_Event_ins_set_pedal(Reltime* pos);


#endif // K_EVENT_INS_SET_PEDAL_H


