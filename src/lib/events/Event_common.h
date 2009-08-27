

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


#define Event_check_int64_t_range Event_check_integral_range


#define create_set_primitive(etype, etype_id, ftype, fname)              \
    static bool etype ## _set(Event* event, int index, void* data);      \
    static bool etype ## _set(Event* event, int index, void* data)       \
    {                                                                    \
        assert(event != NULL);                                           \
        assert(event->type == etype_id);                                 \
        assert(data != NULL);                                            \
        etype* event_sub = (etype*)event;                                \
        if (index == 0)                                                  \
        {                                                                \
            ftype num = *(ftype*)data;                                   \
            Event_check_ ## ftype ## _range(num, event->field_types[0]); \
            event_sub->fname = num;                                      \
            return true;                                                 \
        }                                                                \
        return false;                                                    \
    }


#define create_set_reltime(etype, etype_id, fname)                  \
    static bool etype ## _set(Event* event, int index, void* data); \
    static bool etype ## _set(Event* event, int index, void* data)  \
    {                                                               \
        assert(event != NULL);                                      \
        assert(event->type == etype_id);                            \
        assert(data != NULL);                                       \
        etype* event_sub = (etype*)event;                           \
        if (index == 0)                                             \
        {                                                           \
            Reltime* rt = data;                                     \
            Event_check_reltime_range(rt, event->field_types[0]);   \
            Reltime_copy(&event_sub->fname, rt);                    \
            return true;                                            \
        }                                                           \
        return false;                                               \
    }


#define create_get(etype, etype_id, fname)               \
    static void* etype ## _get(Event* event, int index); \
    static void* etype ## _get(Event* event, int index)  \
    {                                                    \
        assert(event != NULL);                           \
        assert(event->type == etype_id);                 \
        etype* event_sub = (etype*)event;                \
        if (index == 0)                                  \
        {                                                \
            return &event_sub->fname;                    \
        }                                                \
        return NULL;                                     \
    }


#define create_set_primitive_and_get(type, type_id, field_type, field_name) \
    create_set_primitive(type, type_id, field_type, field_name)             \
    create_get(type, type_id, field_name)


#define create_set_reltime_and_get(type, type_id, field_name) \
    create_set_reltime(type, type_id, field_name)             \
    create_get(type, type_id, field_name)


#endif // K_EVENT_COMMON_H


