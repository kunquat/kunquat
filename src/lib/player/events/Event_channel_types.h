

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
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

#ifndef EVENT_CHANNEL_NS_DEF
#define EVENT_CHANNEL_NS_DEF(n, t, a, v, ns) EVENT_CHANNEL_DEF(n, t, a, v)
#endif


//                Name      Type suffix                 Arg type        Validator       [Name setter]
EVENT_CHANNEL_DEF(".a",     set_au_input,               INT,            v_au)

EVENT_CHANNEL_DEF("n+",     note_on,                    FLOAT,          v_pitch)
EVENT_CHANNEL_DEF("h",      hit,                        INT,            v_hit)
EVENT_CHANNEL_DEF("n-",     note_off,                   NONE,           NULL)

EVENT_CHANNEL_DEF(".f",     set_force,                  FLOAT,          v_force)
EVENT_CHANNEL_DEF("/f",     slide_force,                FLOAT,          v_force)
EVENT_CHANNEL_DEF("/=f",    slide_force_length,         TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("ts",     tremolo_speed,              FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("td",     tremolo_depth,              FLOAT,          v_tremolo_depth)
EVENT_CHANNEL_DEF("t/=s",   tremolo_speed_slide,        TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("t/=d",   tremolo_depth_slide,        TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("->f+",   carry_force_on,             NONE,           NULL)
EVENT_CHANNEL_DEF("->f-",   carry_force_off,            NONE,           NULL)

EVENT_CHANNEL_DEF("/p",     slide_pitch,                FLOAT,          v_pitch)
EVENT_CHANNEL_DEF("/=p",    slide_pitch_length,         TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("vs",     vibrato_speed,              FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("vd",     vibrato_depth,              FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("v/=s",   vibrato_speed_slide,        TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("v/=d",   vibrato_depth_slide,        TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("->p+",   carry_pitch_on,             NONE,           NULL)
EVENT_CHANNEL_DEF("->p-",   carry_pitch_off,            NONE,           NULL)

EVENT_CHANNEL_DEF("<arp",   reset_arpeggio,             NONE,           NULL)
EVENT_CHANNEL_DEF(".arpn",  set_arpeggio_note,          FLOAT,          v_pitch)
EVENT_CHANNEL_DEF(".arpi",  set_arpeggio_index,         INT,            v_arp_index)
EVENT_CHANNEL_DEF(".arps",  set_arpeggio_speed,         FLOAT,          v_arp_speed)
EVENT_CHANNEL_DEF("arp+",   arpeggio_on,                NONE,           NULL)
EVENT_CHANNEL_DEF("arp-",   arpeggio_off,               NONE,           NULL)

EVENT_CHANNEL_NS_DEF(".sn", set_stream_name,            STRING,         v_var_name,     ".sn")
EVENT_CHANNEL_NS_DEF(".s",  set_stream_value,           FLOAT,          v_finite_float, ".sn")
EVENT_CHANNEL_NS_DEF("/s",  slide_stream_target,        FLOAT,          v_finite_float, ".sn")
EVENT_CHANNEL_NS_DEF("/=s", slide_stream_length,        TSTAMP,         v_nonneg_ts,    ".sn")
EVENT_CHANNEL_NS_DEF("os",  stream_osc_speed,           FLOAT,          v_nonneg_float, ".sn")
EVENT_CHANNEL_NS_DEF("od",  stream_osc_depth,           FLOAT,          v_nonneg_float, ".sn")
EVENT_CHANNEL_NS_DEF("o/=s", stream_osc_speed_slide,    TSTAMP,         v_nonneg_ts,    ".sn")
EVENT_CHANNEL_NS_DEF("o/=d", stream_osc_depth_slide,    TSTAMP,         v_nonneg_ts,    ".sn")
EVENT_CHANNEL_NS_DEF("->s+", carry_stream_on,           NONE,           NULL,           ".sn")
EVENT_CHANNEL_NS_DEF("->s-", carry_stream_off,          NONE,           NULL,           ".sn")

EVENT_CHANNEL_DEF(".xc",    set_ch_expression,          MAYBE_STRING,   v_maybe_var_name)
EVENT_CHANNEL_DEF(".x",     set_note_expression,        MAYBE_STRING,   v_maybe_var_name)
EVENT_CHANNEL_DEF("->x+",   carry_note_expression_on,   NONE,           NULL)
EVENT_CHANNEL_DEF("->x-",   carry_note_expression_off,  NONE,           NULL)

EVENT_CHANNEL_DEF(".dn",    set_device_event_name,      STRING,         v_dev_event_name)
EVENT_CHANNEL_NS_DEF("d",   fire_device_event,          MAYBE_REALTIME, v_maybe_finite_rt, ".dn")


#undef EVENT_CHANNEL_DEF
#undef EVENT_CHANNEL_NS_DEF


