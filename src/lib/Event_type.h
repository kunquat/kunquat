

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


#ifndef K_EVENT_TYPE_H
#define K_EVENT_TYPE_H


#include <Real.h>
#include <Reltime.h>

#include <kunquat/Editor.h>


#define EVENT_IS_GENERAL(type) ((type) > EVENT_NONE && (type) < EVENT_GENERAL_LAST)
#define EVENT_IS_GLOBAL(type)  ((type) > EVENT_GENERAL_LAST && (type) < EVENT_GLOBAL_LAST)
#define EVENT_IS_VOICE(type)   ((type) > EVENT_GLOBAL_LAST && (type) < EVENT_VOICE_LAST)
#define EVENT_IS_INS(type)     ((type) > EVENT_VOICE_LAST && (type) < EVENT_INS_LAST)
#define EVENT_IS_CHANNEL(type) ((type) > EVENT_INS_LAST && (type) < EVENT_LAST)
#define EVENT_IS_VALID(type)   (EVENT_IS_GENERAL((type)) || \
                                EVENT_IS_GLOBAL((type))  || \
                                EVENT_IS_VOICE((type))   || \
                                EVENT_IS_INS((type))     || \
                                EVENT_IS_CHANNEL((type)))


typedef enum
{
    EVENT_FIELD_NONE = 0,
    EVENT_FIELD_INT,
    EVENT_FIELD_NOTE,
    EVENT_FIELD_NOTE_MOD,
    EVENT_FIELD_DOUBLE,
    EVENT_FIELD_REAL,
    EVENT_FIELD_RELTIME,
    EVENT_FIELD_LAST
} Event_field_type;


/**
 * This is a convenience structure designed to facilitate user interface
 * programming (editing of all Events with the same code).
 *
 * An Event_field_desc contains the format and valid range of a field in an
 * Event.
 */
typedef struct Event_field_desc
{
    Event_field_type type;
    union
    {
        struct
        {
            int64_t min;
            int64_t max;
        } integral_type; ///< Used for int and note(_mod) types.
        struct
        {
            double min;
            double max;
        } double_type;
        struct
        {
            Real min;
            Real max;
        } Real_type;
        struct
        {
            Reltime min;
            Reltime max;
        } Reltime_type;
    } range;
} Event_field_desc;


#define Event_check_integral_range(num, field_desc)       \
    if (true)                                             \
    {                                                     \
        if ((num) < (field_desc).range.integral_type.min) \
        {                                                 \
            return false;                                 \
        }                                                 \
        if ((num) > (field_desc).range.integral_type.max) \
        {                                                 \
            return false;                                 \
        }                                                 \
    } else (void)0


#define Event_check_double_range(num, field_desc)       \
    if (true)                                           \
    {                                                   \
        if ((num) < (field_desc).range.double_type.min) \
        {                                               \
            return false;                               \
        }                                               \
        if ((num) > (field_desc).range.double_type.max) \
        {                                               \
            return false;                               \
        }                                               \
    } else (void)0


#define Event_check_real_range(num, field_desc)                     \
    if (true)                                                       \
    {                                                               \
        if (Real_cmp((num), &(field_desc).range.Real_type.min) < 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
        if (Real_cmp((num), &(field_desc).range.Real_type.max) > 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
    } else (void)0


#define Event_check_reltime_range(num, field_desc)                        \
    if (true)                                                             \
    {                                                                     \
        if (Reltime_cmp((num), &(field_desc).range.Reltime_type.min) < 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
        if (Reltime_cmp((num), &(field_desc).range.Reltime_type.max) > 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
    } else (void)0


#endif // K_EVENT_TYPE_H


