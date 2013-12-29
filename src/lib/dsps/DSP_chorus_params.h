

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef CHORUS_PARAM
#error "CHORUS_PARAM(name, dev_key, update_key, def_value) not defined"
#endif


CHORUS_PARAM(delay, "voice_XX/p_f_delay.json", "v_XX/d", -1.0)
CHORUS_PARAM(range, "voice_XX/p_f_range.json", "v_XX/r", 0.0)
CHORUS_PARAM(speed, "voice_XX/p_f_speed.json", "v_XX/s", 0.0)
CHORUS_PARAM(volume, "voice_XX/p_f_volume.json", "v_XX/v", 0.0)


#undef CHORUS_PARAM


