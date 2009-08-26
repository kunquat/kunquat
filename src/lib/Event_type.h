

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
    EVENT_NONE = 0,                         ///< An uninitialised event.
    EVENT_GENERAL_COND,                     ///< Evaluate a conditional expression.
    EVENT_GENERAL_LAST               =  63, ///< Sentinel -- never used as a valid type.
                                     
    EVENT_GLOBAL_SET_TEMPO           =  64, ///< Set tempo. (BPM (float))
    EVENT_GLOBAL_SLIDE_TEMPO         =  65, ///< Slide tempo.
    EVENT_GLOBAL_SLIDE_TEMPO_LENGTH  =  66,
    EVENT_GLOBAL_PATTERN_DELAY       =  67, ///< Pattern delay.
                                     
    EVENT_GLOBAL_SET_VOLUME          =  72, ///< Set global volume.
    EVENT_GLOBAL_SLIDE_VOLUME        =  73, ///< Slide global volume.
                                     
    EVENT_GLOBAL_SET_SCALE           =  80, ///< Set default scale used by Instruments.
    EVENT_GLOBAL_RETUNE_SCALE        =  81, ///< Retune scale.
                                     
    EVENT_GLOBAL_SET_VAR,                   ///< Set a variable.
                                     
    EVENT_GLOBAL_LAST                = 127, ///< Sentinel -- never used as a valid type.
                                     
    EVENT_VOICE_NOTE_ON              = 128, ///< Note On event. (note, modifier, octave)
    EVENT_VOICE_NOTE_OFF             = 129, ///< Note Off event.
                                     
    EVENT_VOICE_SET_FORCE            = 136, ///< Set Force.
    EVENT_VOICE_SLIDE_FORCE          = 137, ///< Slide Force.
    EVENT_VOICE_SLIDE_FORCE_LENGTH   = 138,
    EVENT_VOICE_TREMOLO_SPEED        = 139, ///< Tremolo speed.
    EVENT_VOICE_TREMOLO_DEPTH        = 140, ///< Tremolo depth.
    EVENT_VOICE_TREMOLO_DELAY        = 141, ///< Tremolo delay.
                                     
    EVENT_VOICE_SLIDE_PITCH          = 144, ///< Slide pitch.
    EVENT_VOICE_SLIDE_PITCH_LENGTH   = 145,
    EVENT_VOICE_VIBRATO_SPEED        = 146, ///< Vibrato speed.
    EVENT_VOICE_VIBRATO_DEPTH        = 147, ///< Vibrato depth.
    EVENT_VOICE_VIBRATO_DELAY        = 148, ///< Vibrato delay.
    EVENT_VOICE_ARPEGGIO             = 149, ///< Arpeggio (the retro effect).
                                     
    EVENT_VOICE_SET_FILTER           = 152, ///< Set filter cut-off.
    EVENT_VOICE_SLIDE_FILTER         = 153, ///< Slide filter cut-off.
    EVENT_VOICE_SLIDE_FILTER_LENGTH  = 154,
    EVENT_VOICE_AUTOWAH_SPEED        = 155, ///< Auto-wah (filter cut-off oscillation) speed.
    EVENT_VOICE_AUTOWAH_DEPTH        = 156, ///< Auto-wah depth.
    EVENT_VOICE_AUTOWAH_DELAY        = 157, ///< Auto-wah delay.
    EVENT_VOICE_SET_RESONANCE        = 158, ///< Set filter resonance (Q factor).
                                     
    EVENT_VOICE_SET_PANNING          = 160, ///< Set panning position.
    EVENT_VOICE_SLIDE_PANNING        = 161, ///< Slide panning position.
    EVENT_VOICE_SLIDE_PANNING_LENGTH = 162,
                                    
    EVENT_VOICE_LAST                 = 255, ///< Sentinel -- never used as a valid type.
                                    
    EVENT_INS_SET_PEDAL              = 256, ///< Set Instrument pedal.
                                    
    EVENT_INS_LAST                   = 287, ///< Sentinel -- never used as a valid type.
                                    
    EVENT_CHANNEL_SET_INSTRUMENT     = 288, ///< Set Instrument.
                                    
    EVENT_LAST                           ///< Sentinel -- never used as a valid type.
} Event_type;


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


