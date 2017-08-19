

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENT_HANDLER_H
#define KQT_EVENT_HANDLER_H


#include <init/devices/Processor.h>
#include <player/Channel.h>
#include <player/devices/Au_state.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <player/events/Event_interfaces.h>
#include <player/General_state.h>
#include <player/Master_params.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Event_handler Event_handler;


/**
 * Create a new Event handler.
 *
 * \return   The new Event handler if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_handler* new_Event_handler(
        Master_params* master_params,
        Channel** channels,
        Device_states* device_states,
        Au_table* au_table);


/**
 * Retrieve the Event names from the Event handler.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 *
 * \return   The Event names.
 */
const Event_names* Event_handler_get_names(const Event_handler* eh);


/**
 * Register a Control Event processor.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 */
bool Event_handler_set_control_process(
        Event_handler* eh, Event_type type, Event_control_interface* control_process);


/**
 * Register a general Event processor.
 *
 * \param eh                The Event handler -- must not be \c NULL.
 * \param type              The type of the Event -- must be a general Event.
 * \param general_process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_general_process(
        Event_handler* eh, Event_type type, Event_general_interface* general_process);


/**
 * Register a Channel Event processor.
 *
 * \param eh           The Event handler -- must not be \c NULL.
 * \param type         The type of the Event -- must be a Channel Event.
 * \param ch_process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_ch_process(
        Event_handler* eh, Event_type type, Event_channel_interface* ch_process);


/**
 * Register a master event processor.
 *
 * \param eh               The Event handler -- must not be \c NULL.
 * \param type             The type of the event -- must be a master event.
 * \param master_process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_master_process(
        Event_handler* eh, Event_type type, Event_master_interface* master_process);


/**
 * Register an Audio unit Event processor.
 *
 * \param eh           The Event handler -- must not be \c NULL.
 * \param type         The type of the Event -- must be an Audio unit Event.
 * \param au_process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_au_process(
        Event_handler* eh, Event_type type, Event_au_interface* au_process);


/**
 * Trigger an event.
 *
 * \param eh       The Event handler -- must not be \c NULL.
 * \param ch_num   The channel number -- must be >= \c 0 and
 *                 < \c KQT_CHANNELS_MAX.
 * \param name     The event name -- must be a valid name.
 * \param arg      The event argument -- must not be \c NULL.
 *
 * \return   \c true if the Event was triggered successfully, otherwise
 *           \c false.
 */
bool Event_handler_trigger(
        Event_handler* eh, int ch_num, const char* name, const Value* arg);


/**
 * Add a key into all Channel-specific processor parameter dictionaries.
 *
 * \param eh    The Event handler -- must not be \c NULL.
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
//bool Event_handler_add_channel_proc_state_key(
//        Event_handler* eh,
//        const char* key);


/**
 * Destroy an existing Event handler.
 *
 * \param eh   The Event handler, or \c NULL.
 */
void del_Event_handler(Event_handler* eh);


#endif // KQT_EVENT_HANDLER_H


