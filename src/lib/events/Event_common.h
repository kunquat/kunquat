

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


#ifndef K_EVENT_COMMON_H
#define K_EVENT_COMMON_H


#include <stdbool.h>

#include <Event.h>
#include <Reltime.h>
#include <xmemory.h>


/**
 * Initialises the Event with necessary high-level information.
 *
 * This function sets the default destructor.
 *
 * \param event         The Event -- must not be \c NULL.
 * \param pos           The position of the Event -- must not be \c NULL.
 * \param type          The Event type -- must be valid.
 * \param field_types   A description of the Event field types -- must not be
 *                      \c NULL.
 *
 * \return   The parameter \a event.
 */
Event* Event_init(Event* event,
                  Reltime* pos,
                  Event_type type,
                  Event_field_desc* field_types);


/**
 * The default destructor for Events.
 *
 * This works for any Event that is allocated with a single call of malloc.
 *
 * \param event   The Event -- must not be \c NULL.
 */
void del_Event_default(Event* event);


//#define Event_check_int64_t_range Event_check_integral_range


#define Event_create_constructor(etype, etype_id, ename) \
    Event* new_ ## etype ## _ ## ename(Reltime* pos)     \
    {                                                    \
        assert(pos != NULL);                             \
        etype* event = xalloc(etype);                    \
        if (event == NULL)                               \
        {                                                \
            return NULL;                                 \
        }                                                \
        Event_init((Event*)event,                        \
                   pos,                                  \
                   etype_id,                             \
                   ename ## _desc);                      \
        return (Event*)event;                            \
    } Event* new_ ## ename(Reltime* pos)


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


