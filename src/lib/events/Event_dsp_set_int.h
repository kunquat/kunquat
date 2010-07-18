

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_DSP_SET_INT_H
#define K_EVENT_DSP_SET_INT_H


#include <stdint.h>

#include <Event_dsp.h>
#include <DSP_conf.h>
#include <Reltime.h>


typedef struct Event_dsp_set_int
{
    Event_dsp parent;
} Event_dsp_set_int;


Event* new_Event_dsp_set_int(Reltime* pos);


bool Event_dsp_set_int_process(DSP_conf* dsp_conf, char* fields);


#endif // K_EVENT_DSP_SET_INT_H


