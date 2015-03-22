

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


#ifndef EVENT_PROCESSOR_DEF
#error "EVENT_PROCESSOR_DEF(..) not defined"
#endif


//                  Name        Type suffix             Arg type        Validator
EVENT_PROCESSOR_DEF("g.Bn",     set_bool_name,          STRING,         v_key)
EVENT_PROCESSOR_DEF("g.B",      set_bool,               BOOL,           v_any_bool)
EVENT_PROCESSOR_DEF("g.In",     set_int_name,           STRING,         v_key)
EVENT_PROCESSOR_DEF("g.I",      set_int,                INT,            v_any_int)
EVENT_PROCESSOR_DEF("g.Fn",     set_float_name,         STRING,         v_key)
EVENT_PROCESSOR_DEF("g.F",      set_float,              FLOAT,          v_any_float)
EVENT_PROCESSOR_DEF("g.Tn",     set_tstamp_name,        STRING,         v_key)
EVENT_PROCESSOR_DEF("g.T",      set_tstamp,             TSTAMP,         v_any_ts)


#undef EVENT_PROCESSOR_DEF


