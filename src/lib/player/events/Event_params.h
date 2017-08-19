

/*
 * Author: Tomi Jylhä-Ollila, Finland 2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_PARAMS_H
#define KQT_EVENT_PARAMS_H


#include <decl.h>


// Common event parameters sent to event processing callbacks.
struct Event_params
{
    bool external;
    const Value* arg;
};


#define EVENT_PARAMS_AUTO (&(Event_params){ .external = false, .arg = NULL })


#endif // KQT_EVENT_PARAMS_H


