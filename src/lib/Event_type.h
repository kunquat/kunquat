

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

    EVENT_GENERAL_LOWER              =   0, ///< General Events.
    
    EVENT_GENERAL_COMMENT            =   1, ///< A comment.

    EVENT_GENERAL_IF                 =  11,
    EVENT_GENERAL_END_IF             =  12,

    EVENT_GENERAL_UPPER              = 200,

    EVENT_GLOBAL_LOWER               = 200, ///< Global Events.
                                     
    EVENT_GLOBAL_SET_TEMPO           = 201,
    EVENT_GLOBAL_SLIDE_TEMPO         = 202,
    EVENT_GLOBAL_SLIDE_TEMPO_LENGTH  = 203,
    EVENT_GLOBAL_PATTERN_DELAY       = 204,
                                     
    EVENT_GLOBAL_SET_VOLUME          = 221,
    EVENT_GLOBAL_SLIDE_VOLUME        = 222,
    EVENT_GLOBAL_SLIDE_VOLUME_LENGTH = 223,

    EVENT_GLOBAL_SET_SCALE           = 241,
    EVENT_GLOBAL_SET_SCALE_OFFSET    = 242,
    EVENT_GLOBAL_MIMIC_SCALE         = 243,
    EVENT_GLOBAL_SHIFT_SCALE_INTERVALS = 244,

    EVENT_GLOBAL_SET_JUMP_SUBSONG    = 261,
    EVENT_GLOBAL_SET_JUMP_SECTION    = 262,
    EVENT_GLOBAL_SET_JUMP_ROW        = 263,
    EVENT_GLOBAL_SET_JUMP_COUNTER    = 264,
    EVENT_GLOBAL_JUMP                = 265,
                                     
    EVENT_GLOBAL_SET_VAR,                   ///< Set a variable.
                                     
    EVENT_GLOBAL_UPPER               = 400,

    EVENT_CHANNEL_LOWER              = 400, ///< Channel Events.

    EVENT_CHANNEL_SET_INSTRUMENT     = 401,
    EVENT_CHANNEL_SET_GENERATOR      = 402,
    EVENT_CHANNEL_SET_DSP            = 403,
    EVENT_CHANNEL_SET_DSP_CONTEXT    = 404,

    EVENT_CHANNEL_NOTE_ON            = 421,
    EVENT_CHANNEL_NOTE_OFF           = 422,

    EVENT_CHANNEL_SET_FORCE          = 441,
    EVENT_CHANNEL_SLIDE_FORCE        = 442,
    EVENT_CHANNEL_SLIDE_FORCE_LENGTH = 443,
    EVENT_CHANNEL_TREMOLO_SPEED      = 444,
    EVENT_CHANNEL_TREMOLO_DEPTH      = 445,
    EVENT_CHANNEL_TREMOLO_DELAY      = 446,

    EVENT_CHANNEL_SLIDE_PITCH        = 462,
    EVENT_CHANNEL_SLIDE_PITCH_LENGTH = 463,
    EVENT_CHANNEL_VIBRATO_SPEED      = 464,
    EVENT_CHANNEL_VIBRATO_DEPTH      = 465,
    EVENT_CHANNEL_VIBRATO_DELAY      = 466,
    EVENT_CHANNEL_ARPEGGIO           = 467,

    EVENT_CHANNEL_SET_LOWPASS        = 481,
    EVENT_CHANNEL_SLIDE_LOWPASS      = 482,
    EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH = 483,
    EVENT_CHANNEL_AUTOWAH_SPEED      = 484,
    EVENT_CHANNEL_AUTOWAH_DEPTH      = 485,
    EVENT_CHANNEL_AUTOWAH_DELAY      = 486,

    EVENT_CHANNEL_SET_RESONANCE      = 501,

    EVENT_CHANNEL_SET_PANNING        = 521,
    EVENT_CHANNEL_SLIDE_PANNING      = 522,
    EVENT_CHANNEL_SLIDE_PANNING_LENGTH = 523,

    EVENT_CHANNEL_SET_GEN_BOOL       = 541,
    EVENT_CHANNEL_SET_GEN_INT        = 542,
    EVENT_CHANNEL_SET_GEN_FLOAT      = 543,
    EVENT_CHANNEL_SET_GEN_REAL       = 544,
    EVENT_CHANNEL_SET_GEN_RELTIME    = 545,

    EVENT_CHANNEL_SET_INS_DSP_BOOL   = 561,
    EVENT_CHANNEL_SET_INS_DSP_INT    = 562,
    EVENT_CHANNEL_SET_INS_DSP_FLOAT  = 563,
    EVENT_CHANNEL_SET_INS_DSP_REAL   = 564,
    EVENT_CHANNEL_SET_INS_DSP_RELTIME = 565,

    EVENT_CHANNEL_UPPER              = 800,

    EVENT_INS_LOWER                  = 800, ///< Instrument Events
                                    
    EVENT_INS_SET_PEDAL              = 801,
                                    
    EVENT_INS_UPPER                  = 900,

    EVENT_GENERATOR_LOWER            = 900, ///< Generator Events

    EVENT_GENERATOR_SET_BOOL         = 901,
    EVENT_GENERATOR_SET_INT          = 902,
    EVENT_GENERATOR_SET_FLOAT        = 903,
    EVENT_GENERATOR_SET_REAL         = 904,
    EVENT_GENERATOR_SET_RELTIME      = 905,

    EVENT_GENERATOR_UPPER            = 1000,

    EVENT_DSP_LOWER                  = 1000, ///< DSP Events

    EVENT_DSP_SET_BOOL               = 1001,
    EVENT_DSP_SET_INT                = 1002,
    EVENT_DSP_SET_FLOAT              = 1003,
    EVENT_DSP_SET_REAL               = 1004,
    EVENT_DSP_SET_RELTIME            = 1005,

    EVENT_DSP_UPPER                  = 1100,
                                    
    EVENT_LAST
} Event_type;


#define EVENT_IS_GENERAL(type)   ((type) > EVENT_GENERAL_LOWER && \
                                  (type) < EVENT_GENERAL_UPPER)
#define EVENT_IS_GLOBAL(type)    ((type) > EVENT_GLOBAL_LOWER && \
                                  (type) < EVENT_GLOBAL_UPPER)
#define EVENT_IS_CHANNEL(type)   ((type) > EVENT_CHANNEL_LOWER && \
                                  (type) < EVENT_CHANNEL_UPPER)
#define EVENT_IS_INS(type)       ((type) > EVENT_INS_LOWER && \
                                  (type) < EVENT_INS_UPPER)
#define EVENT_IS_GENERATOR(type) ((type) > EVENT_GENERATOR_LOWER && \
                                  (type) < EVENT_GENERATOR_UPPER)
#define EVENT_IS_DSP(type)       ((type) > EVENT_DSP_LOWER && \
                                  (type) < EVENT_DSP_UPPER)
#define EVENT_IS_PG(type)        (EVENT_IS_INS((type)) || \
                                  EVENT_IS_GENERATOR((type)) || \
                                  EVENT_IS_DSP((type)))
#define EVENT_IS_VALID(type)     (EVENT_IS_GENERAL((type))   || \
                                  EVENT_IS_GLOBAL((type))    || \
                                  EVENT_IS_INS((type))       || \
                                  EVENT_IS_GENERATOR((type)) || \
                                  EVENT_IS_DSP((type))       || \
                                  EVENT_IS_CHANNEL((type)))


typedef enum
{
    EVENT_FIELD_NONE = 0,
    EVENT_FIELD_BOOL,
    EVENT_FIELD_INT,
    EVENT_FIELD_NOTE,
    EVENT_FIELD_NOTE_MOD,
    EVENT_FIELD_DOUBLE,
    EVENT_FIELD_REAL,
    EVENT_FIELD_RELTIME,
    EVENT_FIELD_STRING,
    EVENT_FIELD_LAST
} Event_field_type;


typedef struct Event_field
{
    Event_field_type type;
    union
    {
        bool bool_type;
        int64_t integral_type; ///< Used for int and note(_mod) types.
        double double_type;
        Real Real_type;
        Reltime Reltime_type;
    } field;
} Event_field;


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


