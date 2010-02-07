

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <Channel_state.h>
#include <Playdata.h>
#include <Event_type.h>


typedef struct Event_handler Event_handler;


/**
 * Creates a new Event handler.
 *
 * \return   The new Event handler if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_handler* new_Event_handler(Playdata* global_state,
                                 Channel_state** ch_states);


/**
 * Registers a Channel Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param type      The type of the Event -- must be a Channel Event.
 * \param process   The process function -- must not be \c NULL.
 */
void Event_handler_set_ch_process(Event_handler* eh,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, char*));


/**
 * Registers a Global Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param type      The type of the Event -- must be a Global Event.
 * \param process   The process function -- must not be \c NULL.
 */
void Event_handler_set_global_process(Event_handler* eh,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             char*));


/**
 * Registers an Instrument Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param type      The type of the Event -- must be an Instrument Event.
 * \param process   The process function -- must not be \c NULL.
 */
#if 0
void Event_handler_set_ins_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*ins_process)(Ins_state*, char*));
#endif


/**
 * Handles an Event.
 *
 * \param eh       The Event handler -- must not be \c NULL.
 * \param ch       The channel number -- must be >= \c -1 and
 *                 < \c KQT_COLUMNS_MAX. \c -1 indicates global state.
 * \param type     The type of the Event -- must be a valid type and
 *                 compatible with the channel number.
 * \param fields   Event fields, or \c NULL if not applicable.
 *
 * \return   \c true if the Event was handled, or \c false if arguments were
 *           invalid.
 */
bool Event_handler_handle(Event_handler* eh,
                          int ch,
                          Event_type type,
                          char* fields);


/**
 * Returns the global state inside the Event handler.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 *
 * \return   The global state.
 */
Playdata* Event_handler_get_global_state(Event_handler* eh);


/**
 * Destroys an existing Event handler.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 */
void del_Event_handler(Event_handler* eh);


#endif // K_EVENT_HANDLER_H


