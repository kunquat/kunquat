

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


#ifndef EVENT_AU_DEF
#error "EVENT_AU_DEF(..) not defined"
#endif


//           Name           Type suffix                 Arg type    Validator
EVENT_AU_DEF("abp+",        bypass_on,                  NONE,       NULL)
EVENT_AU_DEF("abp-",        bypass_off,                 NONE,       NULL)
EVENT_AU_DEF("a.sus",       set_sustain,                FLOAT,      v_sustain)

EVENT_AU_DEF("a.cv",        set_cv_value,               REALTIME,   v_finite_rt)
EVENT_AU_DEF("a/cv",        slide_cv_target,            FLOAT,      v_finite_float)
EVENT_AU_DEF("a/=cv",       slide_cv_length,            TSTAMP,     v_nonneg_ts)
EVENT_AU_DEF("aoscv",       osc_speed_cv,               FLOAT,      v_nonneg_float)
EVENT_AU_DEF("aodcv",       osc_depth_cv,               FLOAT,      v_nonneg_float)
EVENT_AU_DEF("ao/=scv",     osc_speed_slide_cv,         TSTAMP,     v_nonneg_ts)
EVENT_AU_DEF("ao/=dcv",     osc_depth_slide_cv,         TSTAMP,     v_nonneg_ts)


#undef EVENT_AU_DEF


