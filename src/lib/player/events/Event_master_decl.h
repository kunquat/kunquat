

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
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

#include <player/Master_params.h>
#include <Value.h>


// Process function declarations

#define EVENT_TYPE_DEF(type) \
    bool Event_master_##type##_process(Master_params* master_params, const Value* value);
#include <player/events/Event_master_types.h>


#endif // K_EVENT_MASTER_DECL_H


