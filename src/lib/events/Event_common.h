

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
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


#define Event_create_set_primitive(etype, etype_id, ftype, fname)        \
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


#define Event_create_set_reltime(etype, etype_id, fname)            \
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


#define Event_create_get(etype, etype_id, fname)         \
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


#define Event_create_set_primitive_and_get(type, type_id, field_type, field_name) \
    Event_create_set_primitive(type, type_id, field_type, field_name)             \
    Event_create_get(type, type_id, field_name)


#define Event_create_set_reltime_and_get(type, type_id, field_name) \
    Event_create_set_reltime(type, type_id, field_name)             \
    Event_create_get(type, type_id, field_name)


#define Event_create_constructor(etype, etype_id, field_desc, ...) \
    Event* new_ ## etype(Reltime* pos)                             \
    {                                                              \
        assert(pos != NULL);                                       \
        etype* event = xalloc(etype);                              \
        if (event == NULL)                                         \
        {                                                          \
            return NULL;                                           \
        }                                                          \
        Event_init(&event->parent.parent,                          \
                   pos,                                            \
                   etype_id,                                       \
                   field_desc,                                     \
                   etype ## _set,                                  \
                   etype ## _get);                                 \
        event->parent.process = etype ## _process;                 \
        __VA_ARGS__;                                               \
        return (Event*)event;                                      \
    }


#endif // K_EVENT_COMMON_H


