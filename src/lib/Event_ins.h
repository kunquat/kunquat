

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


#ifndef K_EVENT_INS_H
#define K_EVENT_INS_H


#include <Event_pg.h>
#include <Instrument_params.h>


typedef struct Event_ins
{
    Event_pg parent;
    Instrument_params* ins_params;
    void (*process)(struct Event_ins* event);
} Event_ins;


/**
 * Processes the Instrument event.
 *
 * \param event        The Instrument event -- must not be \c NULL.
 */
void Event_ins_process(Event_ins* event);


/**
 * Sets the Instrument parameters for the Instrument event.
 *
 * \param event        The Instrument event -- must not be \c NULL.
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 */
void Event_ins_set_params(Event_ins* event, Instrument_params* ins_params);


#endif // K_EVENT_INS_H


