

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
EVENT_CHANNEL_DEF(".i",     set_ins_input,          INT,            v_ins)
EVENT_CHANNEL_DEF(".g",     set_generator,          INT,            v_gen)
EVENT_CHANNEL_DEF(".e",     set_effect,             INT,            v_effect)
EVENT_CHANNEL_DEF(".ge",    set_global_effects,     NONE,           NULL)
EVENT_CHANNEL_DEF(".ie",    set_instrument_effects, NONE,           NULL)

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

EVENT_CHANNEL_DEF("/p",     slide_pitch,            FLOAT,          v_pitch)
EVENT_CHANNEL_DEF("/=p",    slide_pitch_length,     TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("vs",     vibrato_speed,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("vd",     vibrato_depth,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("v/=s",   vibrato_speed_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("v/=d",   vibrato_depth_slide,    TSTAMP,         v_nonneg_ts)

EVENT_CHANNEL_DEF("<arp",   reset_arpeggio,         NONE,           NULL)
EVENT_CHANNEL_DEF(".arpn",  set_arpeggio_note,      FLOAT,          v_pitch)
EVENT_CHANNEL_DEF(".arpi",  set_arpeggio_index,     INT,            v_arp_index)
EVENT_CHANNEL_DEF(".arps",  set_arpeggio_speed,     FLOAT,          v_arp_speed)
EVENT_CHANNEL_DEF("arp+",   arpeggio_on,            NONE,           NULL)
EVENT_CHANNEL_DEF("arp-",   arpeggio_off,           NONE,           NULL)

EVENT_CHANNEL_DEF(".l",     set_lowpass,            FLOAT,          v_lowpass)
EVENT_CHANNEL_DEF("/l",     slide_lowpass,          FLOAT,          v_lowpass)
EVENT_CHANNEL_DEF("/=l",    slide_lowpass_length,   TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("ws",     autowah_speed,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("wd",     autowah_depth,          FLOAT,          v_nonneg_float)
EVENT_CHANNEL_DEF("w/=s",   autowah_speed_slide,    TSTAMP,         v_nonneg_ts)
EVENT_CHANNEL_DEF("w/=d",   autowah_depth_slide,    TSTAMP,         v_nonneg_ts)

EVENT_CHANNEL_DEF(".r",     set_resonance,          FLOAT,          v_resonance)
//EVENT_CHANNEL_DEF("/r",     slide_resonance,        FLOAT,          v_resonance)
//EVENT_CHANNEL_DEF("/=r",    slide_resonance_length, TSTAMP,         v_nonneg_ts)

EVENT_CHANNEL_DEF(".P",     set_panning,            FLOAT,          v_panning)
EVENT_CHANNEL_DEF("/P",     slide_panning,          FLOAT,          v_panning)
EVENT_CHANNEL_DEF("/=P",    slide_panning_length,   TSTAMP,         v_nonneg_ts)

EVENT_CHANNEL_DEF(".gBn",   set_gen_bool_name,      STRING,         v_key)
EVENT_CHANNEL_DEF(".gB",    set_gen_bool,           BOOL,           v_any_bool)
EVENT_CHANNEL_DEF(".gIn",   set_gen_int_name,       STRING,         v_key)
EVENT_CHANNEL_DEF(".gI",    set_gen_int,            INT,            v_any_int)
EVENT_CHANNEL_DEF(".gFn",   set_gen_float_name,     STRING,         v_key)
EVENT_CHANNEL_DEF(".gF",    set_gen_float,          FLOAT,          v_any_float)
EVENT_CHANNEL_DEF(".gTn",   set_gen_tstamp_name,    STRING,         v_key)
EVENT_CHANNEL_DEF(".gT",    set_gen_tstamp,         TSTAMP,         v_any_ts)


#undef EVENT_CHANNEL_DEF


