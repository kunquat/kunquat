

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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


#include <Real.h>
#include <Tstamp.h>


typedef enum
{
    Event_NONE = 0, ///< An uninitialised event.

    Event_control_START,
#define EVENT_TYPE_DEF(type) Event_control_##type,
#include <events/Event_control_types.h>
    Event_control_STOP,

    Event_general_START,
#define EVENT_TYPE_DEF(type) Event_general_##type,
#include <events/Event_general_types.h>
    Event_general_STOP,

    Event_master_START,
#define EVENT_TYPE_DEF(type) Event_master_##type,
#include <events/Event_master_types.h>

    Trigger_jump, // TODO: handle cleanly

    Event_master_STOP,

    Event_channel_START,
#define EVENT_TYPE_DEF(type) Event_channel_##type,
#include <events/Event_channel_types.h>
    Event_channel_STOP,

    Event_ins_START,
#define EVENT_TYPE_DEF(type) Event_ins_##type,
#include <events/Event_ins_types.h>
    Event_ins_STOP,

    Event_generator_START,
#define EVENT_TYPE_DEF(type) Event_generator_##type,
#include <events/Event_generator_types.h>
    Event_generator_STOP,

    Event_effect_START,
#define EVENT_TYPE_DEF(type) Event_effect_##type,
#include <events/Event_effect_types.h>
    Event_effect_STOP,

    Event_dsp_START,
#define EVENT_TYPE_DEF(type) Event_dsp_##type,
#include <events/Event_dsp_types.h>
    Event_dsp_STOP,

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
#define Event_is_effect(type)    ((type) > Event_effect_START && \
                                  (type) < Event_effect_STOP)
#define Event_is_dsp(type)       ((type) > Event_dsp_START && \
                                  (type) < Event_dsp_STOP)
#define Event_is_query(type)     ((type) > Event_query_START && \
                                  (type) < Event_query_STOP)
#define Event_is_auto(type)      ((type) > Event_auto_START && \
                                  (type) < Event_auto_STOP)
#define Event_is_trigger(type)   (Event_is_ins((type))       || \
                                  Event_is_general((type))   || \
                                  Event_is_generator((type)) || \
                                  Event_is_master((type))    || \
                                  Event_is_effect((type))    || \
                                  Event_is_dsp((type))       || \
                                  Event_is_channel((type))   || \
                                  Event_is_control((type))   || \
                                  Event_is_query((type)))
#define Event_is_valid(type)     (Event_is_trigger((type)) || \
                                  Event_is_auto((type)))


#endif // K_EVENT_TYPE_H


