

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


#ifndef EVENT_GENERAL_DEF
#error "EVENT_GENERAL_DEF(..) not defined"
#endif

#ifndef EVENT_GENERAL_NS_DEF
#define EVENT_GENERAL_NS_DEF(n, t, a, v, ns) EVENT_GENERAL_DEF(n, t, a, v)
#endif


//                Name          Type suffix     Arg type        Validator       [Name spec]
EVENT_GENERAL_DEF("#",          comment,        STRING,         v_any_str)

EVENT_GENERAL_DEF("?",          cond,           BOOL,           v_any_bool)
EVENT_GENERAL_DEF("?if",        if,             NONE,           NULL)
EVENT_GENERAL_DEF("?else",      else,           NONE,           NULL)
EVENT_GENERAL_DEF("?end",       end_if,         NONE,           NULL)

//EVENT_GENERAL_DEF("signal",   signal,         STRING,         v_any_str)
EVENT_GENERAL_DEF("calln",      call_name,      STRING,         v_any_str)
EVENT_GENERAL_NS_DEF("call",    call,           REALTIME,       NULL,           "calln")


#undef EVENT_GENERAL_DEF
#undef EVENT_GENERAL_NS_DEF


