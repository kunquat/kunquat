

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_EVENT_COMMON_H
#define K_EVENT_COMMON_H


#include <stdbool.h>

#include <Event.h>
#include <Reltime.h>


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
 * \param set           Field setter -- must not be \c NULL.
 * \param get           Field getter -- must not be \c NULL.
 *
 * \return   The parameter \a event.
 */
Event* Event_init(Event* event,
                  Reltime* pos,
                  Event_type type,
                  Event_field_desc* field_types,
                  bool (*set)(Event*, int, void*),
                  void* (*get)(Event*, int));


/**
 * The default destructor for Events.
 *
 * This works for any Event that is allocated with a single call of malloc.
 *
 * \param event   The Event -- must not be \c NULL.
 */
void del_Event_default(Event* event);


#endif // K_EVENT_COMMON_H


