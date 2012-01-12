

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
    EVENT_NONE = 0, ///< An uninitialised event.

    EVENT_CONTROL_LOWER, ///< Control Events.

    EVENT_CONTROL_PAUSE,
    EVENT_CONTROL_RESUME,
    EVENT_CONTROL_PLAY_PATTERN,
    EVENT_CONTROL_TEMPO_FACTOR,

    EVENT_CONTROL_ENV_SET_BOOL_NAME,
    EVENT_CONTROL_ENV_SET_BOOL,
    EVENT_CONTROL_ENV_SET_INT_NAME,
    EVENT_CONTROL_ENV_SET_INT,
    EVENT_CONTROL_ENV_SET_FLOAT_NAME,
    EVENT_CONTROL_ENV_SET_FLOAT,
    EVENT_CONTROL_ENV_SET_REAL_NAME,
    EVENT_CONTROL_ENV_SET_REAL,
    EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME,
    EVENT_CONTROL_ENV_SET_TIMESTAMP,

    EVENT_CONTROL_SET_GOTO_SUBSONG,
    EVENT_CONTROL_SET_GOTO_SECTION,
    EVENT_CONTROL_SET_GOTO_ROW,
    EVENT_CONTROL_GOTO,

    EVENT_CONTROL_TURING,

    EVENT_CONTROL_RECEIVE_EVENT,

    EVENT_CONTROL_UPPER,

    EVENT_GENERAL_LOWER, ///< General Events.

    EVENT_GENERAL_COMMENT, ///< A comment.

    EVENT_GENERAL_COND,         ///< Evaluate a conditional expression.
    EVENT_GENERAL_IF,           ///< Start conditional event handling.
    EVENT_GENERAL_END_IF,       ///< End conditional event handling.

    EVENT_GENERAL_SIGNAL,
    EVENT_GENERAL_CALL_BOOL_NAME,
    EVENT_GENERAL_CALL_BOOL,
    EVENT_GENERAL_CALL_INT_NAME,
    EVENT_GENERAL_CALL_INT,
    EVENT_GENERAL_CALL_FLOAT_NAME,
    EVENT_GENERAL_CALL_FLOAT,

    EVENT_GENERAL_UPPER,

    EVENT_GLOBAL_LOWER, ///< Global Events.

    EVENT_GLOBAL_SET_TEMPO,
    EVENT_GLOBAL_SLIDE_TEMPO,
    EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
    EVENT_GLOBAL_PATTERN_DELAY,

    EVENT_GLOBAL_SET_VOLUME,
    EVENT_GLOBAL_SLIDE_VOLUME,
    EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,

    EVENT_GLOBAL_SET_SCALE,
    EVENT_GLOBAL_SET_SCALE_OFFSET,
    EVENT_GLOBAL_MIMIC_SCALE,
    EVENT_GLOBAL_SET_SCALE_FIXED_POINT,
    EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,

    EVENT_GLOBAL_SET_JUMP_SUBSONG,
    EVENT_GLOBAL_SET_JUMP_SECTION,
    EVENT_GLOBAL_SET_JUMP_ROW,
    EVENT_GLOBAL_SET_JUMP_COUNTER,
    EVENT_GLOBAL_JUMP,

    EVENT_GLOBAL_SET_VAR, ///< Set a variable.

    EVENT_GLOBAL_UPPER,

    EVENT_CHANNEL_LOWER, ///< Channel Events.

    EVENT_CHANNEL_SET_INSTRUMENT,
    EVENT_CHANNEL_SET_GENERATOR,
    EVENT_CHANNEL_SET_EFFECT,
    EVENT_CHANNEL_SET_GLOBAL_EFFECTS,
    EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS,
    EVENT_CHANNEL_SET_DSP,

    EVENT_CHANNEL_NOTE_ON,
    EVENT_CHANNEL_HIT,
    EVENT_CHANNEL_NOTE_OFF,

    EVENT_CHANNEL_SET_FORCE,
    EVENT_CHANNEL_SLIDE_FORCE,
    EVENT_CHANNEL_SLIDE_FORCE_LENGTH,
    EVENT_CHANNEL_TREMOLO_SPEED,
    EVENT_CHANNEL_TREMOLO_DEPTH,
    EVENT_CHANNEL_TREMOLO_DELAY,

    EVENT_CHANNEL_SLIDE_PITCH,
    EVENT_CHANNEL_SLIDE_PITCH_LENGTH,
    EVENT_CHANNEL_VIBRATO_SPEED,
    EVENT_CHANNEL_VIBRATO_DEPTH,
    EVENT_CHANNEL_VIBRATO_DELAY,

    EVENT_CHANNEL_RESET_ARPEGGIO,
    EVENT_CHANNEL_SET_ARPEGGIO_NOTE,
    EVENT_CHANNEL_SET_ARPEGGIO_INDEX,
    EVENT_CHANNEL_SET_ARPEGGIO_SPEED,
    EVENT_CHANNEL_ARPEGGIO_ON,
    EVENT_CHANNEL_ARPEGGIO_OFF,

    EVENT_CHANNEL_SET_LOWPASS,
    EVENT_CHANNEL_SLIDE_LOWPASS,
    EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH,
    EVENT_CHANNEL_AUTOWAH_SPEED,
    EVENT_CHANNEL_AUTOWAH_DEPTH,
    EVENT_CHANNEL_AUTOWAH_DELAY,

    EVENT_CHANNEL_SET_RESONANCE,
    EVENT_CHANNEL_SLIDE_RESONANCE,
    EVENT_CHANNEL_SLIDE_RESONANCE_LENGTH,

    EVENT_CHANNEL_SET_PANNING,
    EVENT_CHANNEL_SLIDE_PANNING,
    EVENT_CHANNEL_SLIDE_PANNING_LENGTH,

    EVENT_CHANNEL_SET_GEN_BOOL_NAME,
    EVENT_CHANNEL_SET_GEN_BOOL,
    EVENT_CHANNEL_SET_GEN_INT_NAME,
    EVENT_CHANNEL_SET_GEN_INT,
    EVENT_CHANNEL_SET_GEN_FLOAT_NAME,
    EVENT_CHANNEL_SET_GEN_FLOAT,
    EVENT_CHANNEL_SET_GEN_REAL_NAME,
    EVENT_CHANNEL_SET_GEN_REAL,
    EVENT_CHANNEL_SET_GEN_RELTIME_NAME,
    EVENT_CHANNEL_SET_GEN_RELTIME,

#if 0
    EVENT_CHANNEL_SET_INS_DSP_BOOL,
    EVENT_CHANNEL_SET_INS_DSP_INT,
    EVENT_CHANNEL_SET_INS_DSP_FLOAT,
    EVENT_CHANNEL_SET_INS_DSP_REAL,
    EVENT_CHANNEL_SET_INS_DSP_RELTIME,
#endif

    EVENT_CHANNEL_UPPER,

    EVENT_INS_LOWER, ///< Instrument Events

    EVENT_INS_SET_SUSTAIN,

    EVENT_INS_UPPER,

    EVENT_GENERATOR_LOWER, ///< Generator Events

    EVENT_GENERATOR_SET_BOOL_NAME,
    EVENT_GENERATOR_SET_BOOL,
    EVENT_GENERATOR_SET_INT_NAME,
    EVENT_GENERATOR_SET_INT,
    EVENT_GENERATOR_SET_FLOAT_NAME,
    EVENT_GENERATOR_SET_FLOAT,
    EVENT_GENERATOR_SET_REAL_NAME,
    EVENT_GENERATOR_SET_REAL,
    EVENT_GENERATOR_SET_RELTIME_NAME,
    EVENT_GENERATOR_SET_RELTIME,

    EVENT_GENERATOR_UPPER,

    EVENT_EFFECT_LOWER, ///< Effect Events

    EVENT_EFFECT_BYPASS_ON,
    EVENT_EFFECT_BYPASS_OFF,

    EVENT_EFFECT_UPPER,

    EVENT_DSP_LOWER, ///< DSP Events

    EVENT_DSP_SET_BOOL_NAME,
    EVENT_DSP_SET_BOOL,
    EVENT_DSP_SET_INT_NAME,
    EVENT_DSP_SET_INT,
    EVENT_DSP_SET_FLOAT_NAME,
    EVENT_DSP_SET_FLOAT,
    EVENT_DSP_SET_REAL_NAME,
    EVENT_DSP_SET_REAL,
    EVENT_DSP_SET_RELTIME_NAME,
    EVENT_DSP_SET_RELTIME,

    EVENT_DSP_UPPER,

    EVENT_LAST
} Event_type;


#define EVENT_IS_CONTROL(type)   ((type) > EVENT_CONTROL_LOWER && \
                                  (type) < EVENT_CONTROL_UPPER)
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
#define EVENT_IS_EFFECT(type)    ((type) > EVENT_EFFECT_LOWER && \
                                  (type) < EVENT_EFFECT_UPPER)
#define EVENT_IS_DSP(type)       ((type) > EVENT_DSP_LOWER && \
                                  (type) < EVENT_DSP_UPPER)
#define EVENT_IS_PG(type)        (EVENT_IS_INS((type))       || \
                                  EVENT_IS_GENERATOR((type)) || \
                                  EVENT_IS_EFFECT((type))    || \
                                  EVENT_IS_DSP((type))       || \
                                  EVENT_IS_CONTROL((type)))
#define EVENT_IS_TRIGGER(type)   (EVENT_IS_GENERAL((type)) || \
                                  EVENT_IS_GLOBAL((type))  || \
                                  EVENT_IS_PG((type))      || \
                                  EVENT_IS_CHANNEL((type)))
#define EVENT_IS_VALID(type)     EVENT_IS_TRIGGER((type))


typedef enum
{
    EVENT_FIELD_NONE = 0,
    EVENT_FIELD_BOOL,
    EVENT_FIELD_INT,
//    EVENT_FIELD_NOTE,
//    EVENT_FIELD_NOTE_MOD,
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
        char* string_type;
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


