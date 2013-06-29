

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_MASTER_DECL_H
#define K_EVENT_MASTER_DECL_H


#include <stdbool.h>

#include <Playdata.h>
#include <transient/Master_params.h>
#include <Value.h>


// Process function declarations, TODO: Event_global -> Event_master

#define EVENT_TYPE_DEF(type) \
    bool Event_global_##type##_process(Master_params* master_params, Playdata* global_state, Value* value);
#include <events/Event_master_types.h>


#endif // K_EVENT_MASTER_DECL_H


