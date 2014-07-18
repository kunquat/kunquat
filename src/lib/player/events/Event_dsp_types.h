

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


#ifndef EVENT_DSP_DEF
#error "EVENT_DSP_DEF(..) not defined"
#endif


//            Name          Type suffix             Arg type        Validator
EVENT_DSP_DEF("d.Bn",       set_bool_name,          STRING,         v_key)
EVENT_DSP_DEF("d.B",        set_bool,               BOOL,           v_any_bool)
EVENT_DSP_DEF("d.In",       set_int_name,           STRING,         v_key)
EVENT_DSP_DEF("d.I",        set_int,                INT,            v_any_int)
EVENT_DSP_DEF("d.Fn",       set_float_name,         STRING,         v_key)
EVENT_DSP_DEF("d.F",        set_float,              FLOAT,          v_any_float)
EVENT_DSP_DEF("d.Tn",       set_tstamp_name,        STRING,         v_key)
EVENT_DSP_DEF("d.T",        set_tstamp,             TSTAMP,         v_any_ts)


#undef EVENT_DSP_DEF


