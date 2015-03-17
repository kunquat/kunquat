

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_TYPE_H
#define K_EVENT_TYPE_H


typedef enum
{
    Event_NONE = 0, ///< An uninitialised event.

    Event_control_START,
#define EVENT_CONTROL_DEF(name, type_suffix, arg_type, validator) \
        Event_control_##type_suffix,
#include <player/events/Event_control_types.h>
    Event_control_STOP,

    Event_general_START,
#define EVENT_GENERAL_DEF(name, type_suffix, arg_type, validator) \
        Event_general_##type_suffix,
#include <player/events/Event_general_types.h>
    Event_general_STOP,

    Event_master_START,
#define EVENT_MASTER_DEF(name, type_suffix, arg_type, validator) \
        Event_master_##type_suffix,
#include <player/events/Event_master_types.h>

    Event_master_STOP,

    Event_channel_START,
#define EVENT_CHANNEL_DEF(name, type_suffix, arg_type, validator) \
        Event_channel_##type_suffix,
#include <player/events/Event_channel_types.h>
    Event_channel_STOP,

    Event_ins_START,
#define EVENT_INS_DEF(name, type_suffix, arg_type, validator) Event_ins_##type_suffix,
#include <player/events/Event_ins_types.h>
    Event_ins_STOP,

    Event_generator_START,
#define EVENT_GENERATOR_DEF(name, type_suffix, arg_type, validator) \
        Event_generator_##type_suffix,
#include <player/events/Event_generator_types.h>
    Event_generator_STOP,

    Event_query_START,

    Event_query_location,
    Event_query_voice_count,
    Event_query_actual_force,

    Event_query_STOP,

    Event_auto_START,

    Event_auto_location_track,
    Event_auto_location_system,
    Event_auto_location_pattern,
    Event_auto_location_row,
    Event_auto_voice_count,
    Event_auto_actual_force,

    Event_auto_STOP,

    Event_STOP
} Event_type;


#define Event_is_control(type)   ((type) > Event_control_START && \
                                  (type) < Event_control_STOP)
#define Event_is_general(type)   ((type) > Event_general_START && \
                                  (type) < Event_general_STOP)
#define Event_is_master(type)    ((type) > Event_master_START && \
                                  (type) < Event_master_STOP)
#define Event_is_channel(type)   ((type) > Event_channel_START && \
                                  (type) < Event_channel_STOP)
#define Event_is_ins(type)       ((type) > Event_ins_START && \
                                  (type) < Event_ins_STOP)
#define Event_is_generator(type) ((type) > Event_generator_START && \
                                  (type) < Event_generator_STOP)
#define Event_is_query(type)     ((type) > Event_query_START && \
                                  (type) < Event_query_STOP)
#define Event_is_auto(type)      ((type) > Event_auto_START && \
                                  (type) < Event_auto_STOP)
#define Event_is_trigger(type)   (Event_is_ins((type))       || \
                                  Event_is_general((type))   || \
                                  Event_is_generator((type)) || \
                                  Event_is_master((type))    || \
                                  Event_is_channel((type))   || \
                                  Event_is_control((type))   || \
                                  Event_is_query((type)))
#define Event_is_valid(type)     (Event_is_trigger((type)) || \
                                  Event_is_auto((type)))


#endif // K_EVENT_TYPE_H


