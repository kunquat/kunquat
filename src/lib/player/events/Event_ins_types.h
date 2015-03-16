

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


#ifndef EVENT_INS_DEF
#error "EVENT_INS_DEF(..) not defined"
#endif


//            Name          Type suffix             Arg type        Validator
EVENT_INS_DEF("ibp+",       bypass_on,              NONE,           NULL)
EVENT_INS_DEF("ibp-",       bypass_off,             NONE,           NULL)
EVENT_INS_DEF("i.sus",      set_sustain,            FLOAT,          v_sustain)


#undef EVENT_INS_DEF


