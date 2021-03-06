

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2019
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


#include <decl.h>
#include <player/Event_type.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Reserve a Voice group for a given note.
 *
 * \param ch            The Channel -- must not be \c NULL.
 * \param module        The Module -- must not be \c NULL.
 * \param dstates       The Device states -- must not be \c NULL.
 * \param event_type    The Event type -- must be either \c Event_channel_note_on
 *                      or \c Event_channel_hit.
 * \param arg           The event argument -- must not be \c NULL and must have
 *                      the type expected with \a event_type.
 * \param is_external   \c true if the note or hit originates from an
 *                      external event, otherwise \c false.
 *
 * \return   \c true if a Voice group was reserved, otherwise \c false.
 */
bool reserve_voices(
        Channel* ch,
        const Module* module,
        const Device_states* dstates,
        Event_type event_type,
        const Value* arg,
        bool is_external);


/**
 * Initialise a Voice for a processor in an Audio unit.
 *
 * \param ch            The Channel -- must not be \c NULL.
 * \param au            The Audio unit -- must not be \c NULL.
 * \param group_id      The Voice group id.
 * \param proc_state    The processor state -- must not be \c NULL.
 * \param proc_num      The number of the Processor -- must be >= \c 0 and
 *                      < \c KQT_PROCESSORS_MAX.
 * \param rand_seed     The random seed passed to the Voice (NOTE: should be
 *                      the same for every Voice with the same group!)
 *
 * \return   \c true if a Voice was allocated, otherwise \c false (this implies
 *           that the associated Processor uses stateless voice rendering).
 */
bool init_voice(
        Channel* ch,
        Voice* voice,
        const Audio_unit* au,
        uint64_t group_id,
        const Proc_state* proc_state,
        int proc_num,
        uint64_t rand_seed);


#endif // KQT_NOTE_SETUP_H


