

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_TYPE_DEF
#error "EVENT_TYPE_DEF(name, category, type_suffix, arg_type, validator) not defined"
#endif


#define EVENT_TRIGGER_TYPE_DEF EVENT_TYPE_DEF
#include <player/Event_trigger_types.h>

#define EVENT_AUTO_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TYPE_DEF(name, auto, type_suffix, arg_type, validator)
#include <player/Event_auto_types.h>


#undef EVENT_TYPE_DEF


