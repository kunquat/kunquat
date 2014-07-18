

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_CONTROL_DEF
#error "EVENT_CONTROL_DEF(type) not defined"
#endif


//                Name          Type suffix             Arg type        Validator
EVENT_CONTROL_DEF("Ipause",     pause,                  NONE,           NULL)
EVENT_CONTROL_DEF("Iresume",    resume,                 NONE,           NULL)
EVENT_CONTROL_DEF("Ipattern",   play_pattern,           PAT_INST_REF,   v_piref)
EVENT_CONTROL_DEF("Ireceive",   receive_event,          STRING,         NULL)
// tempo factor?

EVENT_CONTROL_DEF("I.Bn",       env_set_bool_name,      STRING,         v_key)
EVENT_CONTROL_DEF("I.B",        env_set_bool,           BOOL,           v_any_bool)
EVENT_CONTROL_DEF("I.In",       env_set_int_name,       STRING,         v_key)
EVENT_CONTROL_DEF("I.I",        env_set_int,            INT,            v_any_int)
EVENT_CONTROL_DEF("I.Fn",       env_set_float_name,     STRING,         v_key)
EVENT_CONTROL_DEF("I.F",        env_set_float,          FLOAT,          v_any_float)
EVENT_CONTROL_DEF("I.Tn",       env_set_tstamp_name,    STRING,         v_key)
EVENT_CONTROL_DEF("I.T",        env_set_tstamp,         TSTAMP,         v_any_ts)

EVENT_CONTROL_DEF("I.gr",       set_goto_row,           TSTAMP,         v_nonneg_ts)
EVENT_CONTROL_DEF("I.gs",       set_goto_section,       INT,            v_system)
EVENT_CONTROL_DEF("I.gss",      set_goto_song,          INT,            v_song)
EVENT_CONTROL_DEF("Ig",         goto,                   NONE,           NULL)

EVENT_CONTROL_DEF("I.infinite", infinite,               BOOL,           v_any_bool)


#undef EVENT_CONTROL_DEF


