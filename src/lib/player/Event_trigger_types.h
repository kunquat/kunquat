

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_TRIGGER_TYPE_DEF
#error "EVENT_TRIGGER_TYPE_DEF(name, category, type_suffix, arg_type, validator) not defined"
#endif


#define EVENT_CONTROL_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, control, type_suffix, arg_type, validator)
#include <player/events/Event_control_types.h>

#define EVENT_GENERAL_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, general, type_suffix, arg_type, validator)
#include <player/events/Event_general_types.h>

#define EVENT_MASTER_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, master, type_suffix, arg_type, validator)
#include <player/events/Event_master_types.h>

#define EVENT_CHANNEL_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, channel, type_suffix, arg_type, validator)
#include <player/events/Event_channel_types.h>

#define EVENT_INS_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, ins, type_suffix, arg_type, validator)
#include <player/events/Event_ins_types.h>

#define EVENT_GENERATOR_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, generator, type_suffix, arg_type, validator)
#include <player/events/Event_generator_types.h>

#define EVENT_QUERY_DEF(name, type_suffix, arg_type, validator) \
    EVENT_TRIGGER_TYPE_DEF(name, query, type_suffix, arg_type, validator)
#include <player/Event_query_types.h>


#undef EVENT_TRIGGER_TYPE_DEF


