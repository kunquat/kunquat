

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_VOICE_SIGNAL_PLAN_H
#define KQT_VOICE_SIGNAL_PLAN_H


#include <decl.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Create a new Voice signal plan.
 *
 * \param dstates        The Device states -- must not be \c NULL.
 * \param thread_count   The number of rendering threads -- must be valid.
 * \param conns          The Connections -- must not be \c NULL.
 *
 * \return   The new Voice signal plan if successful, or \c NULL if memory
 *           allocation failed.
 */
Voice_signal_plan* new_Voice_signal_plan(
        Device_states* dstates, int thread_count, const Connections* conns);


/**
 * Execute the Voice signal plan.
 *
 * \param plan            The Voice signal plan -- must not be \c NULL.
 * \param dstates         The Device states -- must not be \c NULL.
 * \param thread_id       The ID of the rendering thread -- must be valid.
 * \param vgroup          The Voice group -- must not be \c NULL.
 * \param wbs             The Work buffers -- must not be \c NULL.
 * \param buf_start       The start index of buffer areas to be processed
 *                        -- must be less than the buffer size.
 * \param buf_stop        The stop index of buffer areas to be processed
 *                        -- must not be greater than the buffer size.
 * \param tempo           The current tempo -- must be > \c 0.
 * \param enable_mixing   \c true if voice signals should be added to mixed
 *                        outputs, otherwise \c false.
 *
 * \return   The stop index of complete frames rendered to voice buffers. This
 *           is always within range [\a buf_start, \a buf_stop]. If the stop
 *           index is < \a buf_stop, the note has ended.
 */
int32_t Voice_signal_plan_execute(
        Voice_signal_plan* plan,
        Device_states* dstates,
        int thread_id,
        Voice_group* vgroup,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo,
        bool enable_mixing);


/**
 * Destroy an existing Voice signal plan.
 *
 * \param plan   The Voice signal plan, or \c NULL.
 */
void del_Voice_signal_plan(Voice_signal_plan* plan);


#endif // KQT_VOICE_SIGNAL_PLAN_H


