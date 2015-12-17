

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

EVENT_AU_DEF("a.B",         set_cv_bool_value,          BOOL,       v_any_bool)
EVENT_AU_DEF("a.I",         set_cv_int_value,           INT,        v_any_int)
EVENT_AU_DEF("a.F",         set_cv_float_value,         FLOAT,      v_finite_float)
EVENT_AU_DEF("a/F",         slide_cv_float_target,      FLOAT,      v_finite_float)
EVENT_AU_DEF("a/=F",        slide_cv_float_length,      TSTAMP,     v_nonneg_ts)
EVENT_AU_DEF("aosF",        osc_speed_cv_float,         FLOAT,      v_nonneg_float)
EVENT_AU_DEF("aodF",        osc_depth_cv_float,         FLOAT,      v_nonneg_float)
EVENT_AU_DEF("ao/=sF",      osc_speed_slide_cv_float,   TSTAMP,     v_nonneg_ts)
EVENT_AU_DEF("ao/=dF",      osc_depth_slide_cv_float,   TSTAMP,     v_nonneg_ts)
EVENT_AU_DEF("a.T",         set_cv_tstamp_value,        TSTAMP,     v_any_ts)


#undef EVENT_AU_DEF


