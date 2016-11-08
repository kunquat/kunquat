

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_VOICE_GROUP_H
#define KQT_VOICE_GROUP_H


#include <player/Voice.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * A temporary reference of interdependent Voices sent to an Audio unit.
 */
typedef struct Voice_group
{
    int size;
    Voice** voices;
} Voice_group;


#define VOICE_GROUP_AUTO (&(Voice_group){ .size = 0, .voices = NULL })


/**
 * Initialise the Voice group.
 *
 * \param vg        The Voice group -- must not be \c NULL.
 * \param voices    The array of Voices in the Voice pool
 *                  -- must not be \c NULL.
 * \param offset    The starting index of the group -- must be >= \c 0 and
 *                  < \a vp_size.
 * \param vp_size   The Voice pool size -- must be > \c 0.
 *
 * \return   The parameter \a vg.
 */
Voice_group* Voice_group_init(
        Voice_group* vg, Voice** voices, int offset, int vp_size);


/**
 * Get the number of Voices in the Voice group.
 *
 * \param vg   The Voice group -- must not be \c NULL.
 *
 * \return   The number of Voices in the group.
 */
int Voice_group_get_size(const Voice_group* vg);


/**
 * Get the number of active Voices in the Voice group.
 *
 * \param vg   The Voice group -- must not be \c NULL.
 *
 * \return   The number of active Voices.
 */
int Voice_group_get_active_count(const Voice_group* vg);


/**
 * Get a Voice in the Voice group.
 *
 * \param vg      The Voice group -- must not be \c NULL.
 * \param index   The index inside the group -- must be >= \c 0 and
 *                < Voice_group_get_size(\a vg).
 *
 * \return   The Voice.
 */
Voice* Voice_group_get_voice(Voice_group* vg, int index);


/**
 * Find a Voice inside the Voice group with given Processor ID.
 *
 * \param vg        The Voice group -- must not be \c NULL.
 * \param proc_id   The Processor ID.
 *
 * \return   The Voice inside the group if found, otherwise \c NULL.
 */
Voice* Voice_group_get_voice_by_proc(Voice_group* vg, uint32_t proc_id);


/**
 * Process a Voice group inside the Connections.
 *
 * \param vgroup       The Voice group -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param thread_id    The ID of the rendering thread -- must be valid.
 * \param conns        The Connections -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed
 *                     -- must not be negative.
 * \param buf_stop     The stop index of the buffer area to be processed
 *                     -- must not be negative.
 * \param audio_rate   The audio rate -- must be > \c 0.
 * \param tempo        The current tempo -- must be finite and > \c 0.
 *
 * \return   The stop index of complete frames rendered to voice buffers. This
 *           is always within range [\a buf_start, \a buf_stop]. If the stop
 *           index is < \a buf_stop, the note has ended.
 */
int32_t Voice_group_render(
        Voice_group* vgroup,
        Device_states* dstates,
        int thread_id,
        const Connections* conns,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo);


/**
 * Add Voice signals to mixed signal input buffers.
 *
 * \param vgroup      The Voice group -- must not be \c NULL.
 * \param dstates     The Device states -- must not be \c NULL.
 * \param conns       The Connections -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Voice_group_mix(
        Voice_group* vgroup,
        Device_states* dstates,
        const Connections* conns,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Deactivate all Voices in the Voice group.
 *
 * \param vg   The Voice group -- must not be \c NULL.
 */
void Voice_group_deactivate_all(Voice_group* vg);


/**
 * Deactivate Voices in the Voice group that have not been updated.
 *
 * \param vg   The Voice group -- must not be \c NULL.
 */
void Voice_group_deactivate_unreachable(Voice_group* vg);


#endif // KQT_VOICE_GROUP_H


