

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef PROC_TYPE
#error "PROC_TYPE(..) not defined"
#endif


PROC_TYPE(debug)

PROC_TYPE(add)
PROC_TYPE(bitcrusher)
PROC_TYPE(compress)
PROC_TYPE(delay)
PROC_TYPE(envgen)
PROC_TYPE(filter)
PROC_TYPE(force)
PROC_TYPE(freeverb)
PROC_TYPE(gaincomp)
PROC_TYPE(ks)
PROC_TYPE(looper)
PROC_TYPE(mult)
PROC_TYPE(noise)
PROC_TYPE(padsynth)
PROC_TYPE(panning)
PROC_TYPE(pitch)
PROC_TYPE(rangemap)
PROC_TYPE(sample)
PROC_TYPE(slope)
PROC_TYPE(stream)
PROC_TYPE(volume)


#undef PROC_TYPE


