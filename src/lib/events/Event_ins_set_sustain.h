

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_INS_SET_SUSTAIN_H
#define K_EVENT_INS_SET_SUSTAIN_H


#include <Event_ins.h>
#include <Reltime.h>
#include <Value.h>


Event* new_Event_ins_set_sustain(Reltime* pos);


bool Event_ins_set_sustain_process(Instrument_params* state, Value* value);


#endif // K_EVENT_INS_SET_SUSTAIN_H


