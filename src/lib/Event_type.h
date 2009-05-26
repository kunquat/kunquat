

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


typedef enum
{
    EVENT_TYPE_NONE = 0,            ///< An uninitialised event.
    EVENT_TYPE_GENERAL_COND,        ///< Evaluate a conditional expression.
    EVENT_TYPE_GENERAL_LAST  =  63, ///< Sentinel -- never used as a valid type.
    EVENT_TYPE_GLOBAL_SET_VAR,      ///< Set a variable.
    EVENT_TYPE_GLOBAL_SET_TEMPO,    ///< Set tempo. (BPM (float))
    EVENT_TYPE_GLOBAL_SET_VOLUME,   ///< Set global volume.
    EVENT_TYPE_GLOBAL_LAST   = 127, ///< Sentinel -- never used as a valid type.
    EVENT_TYPE_NOTE_ON       = 128, ///< Note On event. (note, modifier, octave, instrument)
    EVENT_TYPE_NOTE_OFF      = 129, ///< Note Off event.
    EVENT_TYPE_LAST                 ///< Sentinel -- never used as a valid type.
} Event_type;


#define EVENT_TYPE_IS_GENERAL(type) ((type) > EVENT_TYPE_NONE && (type) < EVENT_TYPE_GENERAL_LAST)
#define EVENT_TYPE_IS_GLOBAL(type)  ((type) > EVENT_TYPE_GENERAL_LAST && (type) < EVENT_TYPE_GLOBAL_LAST)
#define EVENT_TYPE_IS_VOICE(type)   ((type) > EVENT_TYPE_GLOBAL_LAST && (type) < EVENT_TYPE_LAST)
#define EVENT_TYPE_IS_VALID(type)   (EVENT_TYPE_IS_GENERAL((type)) || EVENT_TYPE_IS_GLOBAL((type)) || EVENT_TYPE_IS_VOICE((type)))


typedef enum
{
    EVENT_FIELD_TYPE_NONE = 0,
    EVENT_FIELD_TYPE_INT,
    EVENT_FIELD_TYPE_NOTE,
    EVENT_FIELD_TYPE_NOTE_MOD,
    EVENT_FIELD_TYPE_DOUBLE,
    EVENT_FIELD_TYPE_REAL,
    EVENT_FIELD_TYPE_RELTIME,
    EVENT_FIELD_TYPE_LAST
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
    do                                                    \
    {                                                     \
        if ((num) < (field_desc).range.integral_type.min) \
        {                                                 \
            return false;                                 \
        }                                                 \
        if ((num) > (field_desc).range.integral_type.max) \
        {                                                 \
            return false;                                 \
        }                                                 \
    } while (false)


#define Event_check_double_range(num, field_desc)       \
    do                                                  \
    {                                                   \
        if ((num) < (field_desc).range.double_type.min) \
        {                                               \
            return false;                               \
        }                                               \
        if ((num) > (field_desc).range.double_type.max) \
        {                                               \
            return false;                               \
        }                                               \
    } while (false)


#define Event_check_real_range(num, field_desc)                     \
    do                                                              \
    {                                                               \
        if (Real_cmp((num), &(field_desc).range.Real_type.min) < 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
        if (Real_cmp((num), &(field_desc).range.Real_type.max) > 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
    } while (false)


#define Event_check_reltime_range(num, field_desc)                        \
    do                                                                    \
    {                                                                     \
        if (Reltime_cmp((num), &(field_desc).range.Reltime_type.min) < 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
        if (Reltime_cmp((num), &(field_desc).range.Reltime_type.max) > 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
    } while (false)


#endif // K_EVENT_TYPE_H


