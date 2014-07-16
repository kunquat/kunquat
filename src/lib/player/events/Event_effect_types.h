

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_EFFECT_DEF
#error "EVENT_EFFECT_DEF(..) not defined"
#endif


//               Name       Type suffix             Arg type        Validator
EVENT_EFFECT_DEF("ebp+",    bypass_on,              NONE,           NULL)
EVENT_EFFECT_DEF("ebp-",    bypass_off,             NONE,           NULL)


#undef EVENT_EFFECT_DEF


