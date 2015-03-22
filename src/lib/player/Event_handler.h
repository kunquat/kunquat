

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


#ifndef K_EVENT_HANDLER_H
#define K_EVENT_HANDLER_H


#include <stdbool.h>

#include <devices/Processor.h>
#include <player/Channel.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <player/General_state.h>
#include <player/Au_state.h>
#include <player/Master_params.h>
#include <Value.h>


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
        Event_handler* eh,
        Event_type type,
        bool (*control_process)(General_state*, const Value*));


/**
 * Register a general Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a general Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_general_process(
        Event_handler* eh,
        Event_type type,
        bool (*general_process)(General_state*, const Value*));


/**
 * Register a Channel Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a Channel Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_ch_process(
        Event_handler* eh,
        Event_type type,
        bool (*ch_process)(Channel*, Device_states*, const Value*));


/**
 * Register a master event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the event -- must be a master event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_master_process(
        Event_handler* eh,
        Event_type type,
        bool (*master_process)(Master_params*, const Value*));


/**
 * Register an Audio unit Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be an Audio unit Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_au_process(
        Event_handler* eh,
        Event_type type,
        bool (*au_process)(
            const Audio_unit*,
            const Au_params*,
            Au_state*,
            Device_states*,
            const Value*));


/**
 * Register a Processor Event processor.
 *
 * \param eh             The Event handler -- must not be \c NULL.
 * \param name           The name of the Event -- must not be \c NULL, empty
 *                       string or longer than EVENT_NAME_MAX characters.
 * \param type           The type of the Event -- must be a Processor Event.
 * \param proc_process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_processor_process(
        Event_handler* eh,
        Event_type type,
        bool (*proc_process)(const Device_impl*, Device_state*, Channel*, const Value*));


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
        Event_handler* eh,
        int ch_num,
        const char* name,
        const Value* arg);


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


#endif // K_EVENT_HANDLER_H


