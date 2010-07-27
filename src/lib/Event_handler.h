

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
#include <DSP_conf.h>
#include <DSP_table.h>
#include <Generator.h>
#include <Ins_table.h>
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
                                 Channel_state** ch_states,
                                 Ins_table* insts,
                                 DSP_table* dsps);


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
void Event_handler_set_ins_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*ins_process)(Instrument_params*, char*));


/**
 * Registers a Generator Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param type      The type of the Event -- must be a Generator Event.
 * \param process   The process function -- must not be \c NULL.
 */
void Event_handler_set_generator_process(Event_handler* eh,
                                         Event_type type,
                                         bool (*gen_process)(Generator*, char*));


/**
 * Registers a DSP Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param type      The type of the Event -- must be a DSP Event.
 * \param process   The process function -- must not be \c NULL.
 */
void Event_handler_set_dsp_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*dsp_process)(DSP_conf*, char*));


/**
 * Handles an Event.
 *
 * \param eh       The Event handler -- must not be \c NULL.
 * \param index    The index number. This must be \c -1 for global Events,
 *                 or >= \c 0 and < \c KQT_COLUMNS_MAX for Channel Events,
 *                 or >= \c 0 and < \c KQT_INSTRUMENTS_MAX for Instrument Events,
 *                 or >= \c 0 and < \c KQT_COLUMNS_MAX for Generator Events.
 * \param type     The type of the Event -- must be a valid type and
 *                 compatible with \a index.
 * \param fields   Event fields, or \c NULL if not applicable.
 *
 * \return   \c true if the Event was handled, or \c false if arguments were
 *           invalid.
 */
bool Event_handler_handle(Event_handler* eh,
                          int index,
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
 * Adds a key into all Channel-specific generator parameter dictionaries.
 *
 * \param eh    The Event handler -- must not be \c NULL.
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_add_channel_gen_state_key(Event_handler* eh,
                                             const char* key);


/**
 * Destroys an existing Event handler.
 *
 * \param eh   The Event handler, or \c NULL.
 */
void del_Event_handler(Event_handler* eh);


#endif // K_EVENT_HANDLER_H


