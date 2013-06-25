

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_TYPE_DEF
#error "EVENT_TYPE_DEF(type) not defined"
#endif


EVENT_TYPE_DEF(pause)
EVENT_TYPE_DEF(resume)
EVENT_TYPE_DEF(play_pattern)
EVENT_TYPE_DEF(tempo_factor)

EVENT_TYPE_DEF(env_set_bool_name)
EVENT_TYPE_DEF(env_set_bool)
EVENT_TYPE_DEF(env_set_int_name)
EVENT_TYPE_DEF(env_set_int)
EVENT_TYPE_DEF(env_set_float_name)
EVENT_TYPE_DEF(env_set_float)
EVENT_TYPE_DEF(env_set_tstamp_name)
EVENT_TYPE_DEF(env_set_tstamp)

EVENT_TYPE_DEF(set_goto_song)
EVENT_TYPE_DEF(set_goto_section)
EVENT_TYPE_DEF(set_goto_row)
EVENT_TYPE_DEF(goto)

EVENT_TYPE_DEF(infinite)

EVENT_TYPE_DEF(receive_event)


#undef EVENT_TYPE_DEF


