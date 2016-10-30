

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_AUTO_DEF
#error "EVENT_AUTO_DEF(..) not defined"
#endif


//             Name         Type suffix             Arg type        Validator
EVENT_AUTO_DEF("Atrack",    location_track,         INT,            v_track)
EVENT_AUTO_DEF("Asystem",   location_system,        INT,            v_system)
EVENT_AUTO_DEF("Apattern",  location_pattern,       PAT_INST_REF,   v_piref)
EVENT_AUTO_DEF("Arow",      location_row,           TSTAMP,         v_any_ts)
EVENT_AUTO_DEF("Avoices",   voice_count,            INT,            v_any_int)
EVENT_AUTO_DEF("Af",        actual_force,           REALTIME,       NULL)


#undef EVENT_AUTO_DEF


