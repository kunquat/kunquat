

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_MASTER_DECL_H
#define KQT_EVENT_MASTER_DECL_H


#include <player/Master_params.h>
#include <Value.h>

#include <stdbool.h>


// Process function declarations

#define EVENT_MASTER_DEF(name, type_suffix, arg_type, validator) \
    bool Event_master_##type_suffix##_process(                   \
            Master_params* master_params, const Value* value);
#include <player/events/Event_master_types.h>


#endif // KQT_EVENT_MASTER_DECL_H


