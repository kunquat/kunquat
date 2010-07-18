

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


#ifndef K_EVENT_GENERATOR_SET_INT_H
#define K_EVENT_GENERATOR_SET_INT_H


#include <Event_generator.h>
#include <Generator.h>
#include <Reltime.h>


typedef struct Event_generator_set_int
{
    Event_generator parent;
} Event_generator_set_int;


Event* new_Event_generator_set_int(Reltime* pos);


bool Event_generator_set_int_process(Generator* gen, char* fields);


#endif // K_EVENT_GENERATOR_SET_INT_H


