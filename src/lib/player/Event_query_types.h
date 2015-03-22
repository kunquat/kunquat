

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_QUERY_DEF
#error "EVENT_QUERY_DEF(..) not defined"
#endif


//              Name            Type suffix         Arg type        Validator
EVENT_QUERY_DEF("qlocation",    location,           NONE,           NULL)
EVENT_QUERY_DEF("qvoices",      voice_count,        NONE,           NULL)
EVENT_QUERY_DEF("qf",           actual_force,       INT,            v_proc)


#undef EVENT_QUERY_DEF


