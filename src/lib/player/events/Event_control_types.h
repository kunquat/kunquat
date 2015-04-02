

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


#ifndef EVENT_CONTROL_DEF
#error "EVENT_CONTROL_DEF(type) not defined"
#endif


//                Name          Type suffix             Arg type        Validator
EVENT_CONTROL_DEF("cpause",     pause,                  NONE,           NULL)
EVENT_CONTROL_DEF("cresume",    resume,                 NONE,           NULL)
EVENT_CONTROL_DEF("cpattern",   play_pattern,           PAT_INST_REF,   v_piref)
// tempo factor?

EVENT_CONTROL_DEF("c.Bn",       env_set_bool_name,      STRING,         v_key)
EVENT_CONTROL_DEF("c.B",        env_set_bool,           BOOL,           v_any_bool)
EVENT_CONTROL_DEF("c.In",       env_set_int_name,       STRING,         v_key)
EVENT_CONTROL_DEF("c.I",        env_set_int,            INT,            v_any_int)
EVENT_CONTROL_DEF("c.Fn",       env_set_float_name,     STRING,         v_key)
EVENT_CONTROL_DEF("c.F",        env_set_float,          FLOAT,          v_any_float)
EVENT_CONTROL_DEF("c.Tn",       env_set_tstamp_name,    STRING,         v_key)
EVENT_CONTROL_DEF("c.T",        env_set_tstamp,         TSTAMP,         v_any_ts)

EVENT_CONTROL_DEF("I.gr",       set_goto_row,           TSTAMP,         v_nonneg_ts)
EVENT_CONTROL_DEF("I.gs",       set_goto_section,       INT,            v_system)
EVENT_CONTROL_DEF("I.gss",      set_goto_song,          INT,            v_song)
EVENT_CONTROL_DEF("Ig",         goto,                   NONE,           NULL)

EVENT_CONTROL_DEF("cinfinite+", infinite_on,            NONE,           NULL)
EVENT_CONTROL_DEF("cinfinite-", infinite_off,           NONE,           NULL)


#undef EVENT_CONTROL_DEF


