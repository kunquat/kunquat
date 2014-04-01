

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


EVENT_TYPE_DEF(set_tempo)
EVENT_TYPE_DEF(slide_tempo)
EVENT_TYPE_DEF(slide_tempo_length)

EVENT_TYPE_DEF(pattern_delay)

EVENT_TYPE_DEF(set_volume)
EVENT_TYPE_DEF(slide_volume)
EVENT_TYPE_DEF(slide_volume_length)

EVENT_TYPE_DEF(set_scale)
EVENT_TYPE_DEF(set_scale_offset)
EVENT_TYPE_DEF(mimic_scale)
EVENT_TYPE_DEF(set_scale_fixed_point)
EVENT_TYPE_DEF(shift_scale_intervals)

EVENT_TYPE_DEF(set_jump_pat_inst)
EVENT_TYPE_DEF(set_jump_row)
EVENT_TYPE_DEF(set_jump_counter)
EVENT_TYPE_DEF(jump)


#undef EVENT_TYPE_DEF


