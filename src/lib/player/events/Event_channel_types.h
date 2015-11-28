

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_CHANNEL_DEF
#error "EVENT_CHANNEL_DEF(..) not defined"
#endif


//                Name      Type suffix             Arg type        Validator
EVENT_CHANNEL_DEF(".a",     set_au_input,           INT,            v_au)

EVENT_CHANNEL_DEF("n+",     note_on,                FLOAT,          v_pitch)
EVENT_CHANNEL_DEF("h",      hit,                    INT,            v_hit)
EVENT_CHANNEL_DEF("n-",     note_off,               NONE,           NULL)

EVENT_CHANNEL_DEF(".f",     set_force,              FLOAT,          v_force)
EVENT_CHANNEL_DEF("/f",     slide_force,            FLOAT,          v_force)
EVENT_CHANNEL_DEF("/=f",    slide_force_length,     TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("ts",     tremolo_speed,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("td",     tremolo_depth,          FLOAT,          v_tremolo_depth)
EVENT_CHANNEL_DEF("t/=s",   tremolo_speed_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("t/=d",   tremolo_depth_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("->f+",   carry_force_on,         NONE,           NULL)
EVENT_CHANNEL_DEF("->f-",   carry_force_off,        NONE,           NULL)

EVENT_CHANNEL_DEF("/p",     slide_pitch,            FLOAT,          v_pitch)
EVENT_CHANNEL_DEF("/=p",    slide_pitch_length,     TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("vs",     vibrato_speed,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("vd",     vibrato_depth,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("v/=s",   vibrato_speed_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("v/=d",   vibrato_depth_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("->p+",   carry_pitch_on,         NONE,           NULL)
EVENT_CHANNEL_DEF("->p-",   carry_pitch_off,        NONE,           NULL)

EVENT_CHANNEL_DEF("<arp",   reset_arpeggio,         NONE,           NULL)
EVENT_CHANNEL_DEF(".arpn",  set_arpeggio_note,      FLOAT,          v_pitch)
EVENT_CHANNEL_DEF(".arpi",  set_arpeggio_index,     INT,            v_arp_index)
EVENT_CHANNEL_DEF(".arps",  set_arpeggio_speed,     FLOAT,          v_arp_speed)
EVENT_CHANNEL_DEF("arp+",   arpeggio_on,            NONE,           NULL)
EVENT_CHANNEL_DEF("arp-",   arpeggio_off,           NONE,           NULL)

EVENT_CHANNEL_DEF(".L",     set_lowpass,            FLOAT,          v_lowpass)
EVENT_CHANNEL_DEF("/L",     slide_lowpass,          FLOAT,          v_lowpass)
EVENT_CHANNEL_DEF("/=L",    slide_lowpass_length,   TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("ws",     autowah_speed,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("wd",     autowah_depth,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("w/=s",   autowah_speed_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("w/=d",   autowah_depth_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF(".r",     set_resonance,          FLOAT,          v_resonance)
EVENT_CHANNEL_DEF("/r",     slide_resonance,        FLOAT,          v_resonance)
EVENT_CHANNEL_DEF("/=r",    slide_resonance_length, TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("->L+",   carry_filter_on,        NONE,           NULL)
EVENT_CHANNEL_DEF("->L-",   carry_filter_off,       NONE,           NULL)

EVENT_CHANNEL_DEF(".P",     set_panning,            FLOAT,          v_panning)
EVENT_CHANNEL_DEF("/P",     slide_panning,          FLOAT,          v_panning)
EVENT_CHANNEL_DEF("/=P",    slide_panning_length,   TSTAMP,         v_nonneg_ts)

EVENT_CHANNEL_DEF(".Bn",    set_cv_bool_name,       STRING,         v_var_name)
EVENT_CHANNEL_DEF(".B",     set_cv_bool_value,      BOOL,           v_any_bool)
EVENT_CHANNEL_DEF(".In",    set_cv_int_name,        STRING,         v_var_name)
EVENT_CHANNEL_DEF(".I",     set_cv_int_value,       INT,            v_any_int)
EVENT_CHANNEL_DEF(".Fn",    set_cv_float_name,      STRING,         v_var_name)
EVENT_CHANNEL_DEF(".F",     set_cv_float_value,     FLOAT,          v_finite_float)
EVENT_CHANNEL_DEF(".Tn",    set_cv_tstamp_name,     STRING,         v_var_name)
EVENT_CHANNEL_DEF(".T",     set_cv_tstamp_value,    TSTAMP,         v_any_ts)


#undef EVENT_CHANNEL_DEF


