

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_NOTE_SETUP_H
#define KQT_NOTE_SETUP_H


#include <init/devices/Audio_unit.h>
#include <player/Channel.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Reserve a Voice for a processor in an Audio unit.
 *
 * \param ch           The Channel -- must not be \c NULL.
 * \param au           The Audio unit -- must not be \c NULL.
 * \param group_id     The Voice group id.
 * \param proc_state   The processor state -- must not be \c NULL.
 * \param proc_num     The number of the Processor -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 * \param rand_seed    The random seed passed to the Voice (NOTE: should be
 *                     the same for every Voice with the same group!)
 */
void reserve_voice(
        Channel* ch,
        Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed);


#endif // KQT_NOTE_SETUP_H


