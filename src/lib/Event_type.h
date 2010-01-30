

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
    EVENT_GLOBAL_SLIDE_VOLUME_LENGTH =  74,
                                     
    EVENT_GLOBAL_SET_SCALE           =  80, ///< Set default scale used by Instruments.
    EVENT_GLOBAL_RETUNE_SCALE        =  81, ///< Retune scale.

    EVENT_GLOBAL_SET_JUMP_SUBSONG    =  88,
    EVENT_GLOBAL_SET_JUMP_SECTION    =  89,
    EVENT_GLOBAL_SET_JUMP_ROW        =  90,
    EVENT_GLOBAL_SET_JUMP_COUNTER    =  91,
    EVENT_GLOBAL_JUMP                =  92,
                                     
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
    EVENT_CHANNEL_LAST                    ,
                                    
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


typedef struct Event_field
{
    Event_field_type type;
    union
    {
        int64_t integral_type; ///< Used for int and note(_mod) types.
        double double_type;
        Real Real_type;
        Reltime Reltime_type;
    } field;
} Event_field;


/**
 * This convenience structure unifies Event field validation among different
 * Event types.
 *
 * An Event_field_desc contains the format and valid range of a field in an
 * Event.
 */
#if 0
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
#endif


typedef struct Event_field_desc
{
    Event_field_type type;
    Event_field min;
    Event_field max;
} Event_field_desc;


#define Event_check_integral_range(num, field_desc)       \
    if (true)                                             \
    {                                                     \
        assert((field_desc).type >= EVENT_FIELD_INT);     \
        assert((field_desc).type < EVENT_FIELD_DOUBLE);   \
        if ((num) < (field_desc).min.field.integral_type) \
        {                                                 \
            return false;                                 \
        }                                                 \
        if ((num) > (field_desc).max.field.integral_type) \
        {                                                 \
            return false;                                 \
        }                                                 \
    } else (void)0


#define Event_check_double_range(num, field_desc)        \
    if (true)                                            \
    {                                                    \
        assert((field_desc).type == EVENT_FIELD_DOUBLE); \
        if ((num) < (field_desc).min.field.double_type)  \
        {                                                \
            return false;                                \
        }                                                \
        if ((num) > (field_desc).max.field.double_type)  \
        {                                                \
            return false;                                \
        }                                                \
    } else (void)0


#define Event_check_real_range(num, field_desc)                     \
    if (true)                                                       \
    {                                                               \
        assert((field_desc).type == EVENT_FIELD_REAL);              \
        if (Real_cmp((num), &(field_desc).min.field.Real_type) < 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
        if (Real_cmp((num), &(field_desc).max.field.Real_type) > 0) \
        {                                                           \
            return false;                                           \
        }                                                           \
    } else (void)0


#define Event_check_reltime_range(num, field_desc)                        \
    if (true)                                                             \
    {                                                                     \
        assert((field_desc).type == EVENT_FIELD_RELTIME);                 \
        if (Reltime_cmp((num), &(field_desc).min.field.Reltime_type) < 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
        if (Reltime_cmp((num), &(field_desc).max.field.Reltime_type) > 0) \
        {                                                                 \
            return false;                                                 \
        }                                                                 \
    } else (void)0


#endif // K_EVENT_TYPE_H


