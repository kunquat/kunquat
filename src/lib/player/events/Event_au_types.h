

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


#ifndef EVENT_AU_DEF
#error "EVENT_AU_DEF(..) not defined"
#endif

#ifndef EVENT_AU_NS_DEF
#define EVENT_AU_NS_DEF(n, t, a, v, ns) EVENT_AU_DEF(n, t, a, v)
#endif


//              Name        Type suffix             Arg type        Validator       [Name setter]
EVENT_AU_DEF(   "abp+",     bypass_on,              NONE,           NULL)
EVENT_AU_DEF(   "abp-",     bypass_off,             NONE,           NULL)
EVENT_AU_DEF(   "a.sus",    set_sustain,            FLOAT,          v_sustain)

EVENT_AU_NS_DEF("a.s",      set_stream_value,       FLOAT,          v_finite_float, ".sn")
EVENT_AU_NS_DEF("a/s",      slide_stream_target,    FLOAT,          v_finite_float, ".sn")
EVENT_AU_NS_DEF("a/=s",     slide_stream_length,    TSTAMP,         v_nonneg_ts,    ".sn")
EVENT_AU_NS_DEF("aos",      stream_osc_speed,       FLOAT,          v_nonneg_float, ".sn")
EVENT_AU_NS_DEF("aod",      stream_osc_depth,       FLOAT,          v_nonneg_float, ".sn")
EVENT_AU_NS_DEF("ao/=s",    stream_osc_speed_slide, TSTAMP,         v_nonneg_ts,    ".sn")
EVENT_AU_NS_DEF("ao/=d",    stream_osc_depth_slide, TSTAMP,         v_nonneg_ts,    ".sn")

EVENT_AU_DEF(   "a.v",      set_cv_value,           REALTIME,       v_finite_rt)

EVENT_AU_NS_DEF("ad",       fire_device_event,      MAYBE_REALTIME, v_maybe_finite_rt, ".dn")


#undef EVENT_AU_DEF
#undef EVENT_AU_NS_DEF


