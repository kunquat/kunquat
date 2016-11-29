

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
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

EVENT_CONTROL_DEF("c.evn",      env_set_var_name,       STRING,         v_key)
EVENT_CONTROL_DEF("c.ev",       env_set_var,            REALTIME,       v_finite_rt)

EVENT_CONTROL_DEF("c.gr",       set_goto_row,           TSTAMP,         v_nonneg_ts)
EVENT_CONTROL_DEF("c.gp",       set_goto_pat_inst,      PAT_INST_REF,   v_piref)
EVENT_CONTROL_DEF("cg",         goto,                   NONE,           NULL)

EVENT_CONTROL_DEF("cinfinite+", infinite_on,            NONE,           NULL)
EVENT_CONTROL_DEF("cinfinite-", infinite_off,           NONE,           NULL)

EVENT_CONTROL_DEF("c.testproc", set_test_processor,     INT,            v_proc)


#undef EVENT_CONTROL_DEF


