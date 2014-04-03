

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


#ifndef K_EVENT_COMMON_H
#define K_EVENT_COMMON_H


#include <stdbool.h>

#include <Tstamp.h>


/**
 * Initialises the Event with necessary high-level information.
 *
 * This function sets the default destructor.
 *
 * \param event         The Event -- must not be \c NULL.
 * \param pos           The position of the Event -- must not be \c NULL.
 * \param type          The Event type -- must be valid.
 *
 * \return   The parameter \a event.
 */
//Event* Event_init(Event* event,
//                  Tstamp* pos,
//                  Event_type type);


/**
 * The default destructor for Events.
 *
 * This works for any Event that does not contain dynamically allocated fields
 * in addition to the event fields.
 *
 * \param event   The Event, or \c NULL.
 */
//void del_Event_default(Event* event);


#define Event_check_voice(ch_state, gen)                        \
    if (true)                                                   \
    {                                                           \
        if ((ch_state)->fg[(gen)] == NULL)                      \
        {                                                       \
            continue;                                           \
        }                                                       \
        (ch_state)->fg[(gen)] =                                 \
                Voice_pool_get_voice((ch_state)->pool,          \
                                     (ch_state)->fg[(gen)],     \
                                     (ch_state)->fg_id[(gen)]); \
        if ((ch_state)->fg[(gen)] == NULL)                      \
        {                                                       \
            continue;                                           \
        }                                                       \
    }                                                           \
    else (void)0


#endif // K_EVENT_COMMON_H


