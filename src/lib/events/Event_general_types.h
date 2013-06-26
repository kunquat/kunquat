

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


EVENT_TYPE_DEF(comment)

EVENT_TYPE_DEF(cond)
EVENT_TYPE_DEF(if)
EVENT_TYPE_DEF(else)
EVENT_TYPE_DEF(end_if)

EVENT_TYPE_DEF(signal)
EVENT_TYPE_DEF(call_bool_name)
EVENT_TYPE_DEF(call_bool)
EVENT_TYPE_DEF(call_int_name)
EVENT_TYPE_DEF(call_int)
EVENT_TYPE_DEF(call_float_name)
EVENT_TYPE_DEF(call_float)


#undef EVENT_TYPE_DEF


