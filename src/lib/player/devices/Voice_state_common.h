

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_STATE_COMMON_H
#define K_VOICE_STATE_COMMON_H


#include <decl.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <player/Audio_buffer.h>
#include <player/devices/Au_state.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


/**
 * Process pitch.
 *
 * This function fills WORK_BUFFER_PITCH_PARAMS and WORK_BUFFER_ACTUAL_PITCHES
 * with up-to-date values in the specified buffer area.
 *
 * \param vstate      The Voice state -- must not be \c NULL.
 * \param proc        The Processor -- must not be \c NULL.
 * \param wbs         The Work buffers -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Voice_state_common_handle_pitch(
        Voice_state* vstate,
        const Processor* proc,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop);


#if 0
/**
 * Process force.
 *
 * This function fills WORK_BUFFER_ACTUAL_FORCES with up-to-date values in the
 * specified buffer area. It must be called after \a Proc_common_handle_pitch.
 *
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param au_state      The Audio unit state -- must not be \c NULL.
 * \param proc          The Processor -- must not be \c NULL.
 * \param wbs           The Work buffers -- must not be \c NULL.
 * \param audio_rate    The audio rate -- must be positive.
 * \param buf_start     The start index of the buffer area to be processed.
 * \param buf_stop      The stop index of the buffer area to be processed.
 *
 * \return   A new buffer stop index that is within the range of
 *           [\a buf_start, \a buf_stop]. Values less than \a buf_stop
 *           indicate that permanent silence is reached and the voice should be
 *           deactivated after the current process cycle.
 */
int32_t Voice_state_common_handle_force(
        Voice_state* vstate,
        const Au_state* au_state,
        const Processor* proc,
        const Work_buffers* wbs,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process volume ramping at the end of a note.
 *
 * This function should be called after the process function of the processor
 * implementation.
 *
 * \param vstate          The Voice state -- must not be \c NULL.
 * \param proc_state      The Processor state -- must not be \c NULL.
 * \param au_state        The Audio unit state -- must not be \c NULL.
 * \param wbs             The Work buffers -- must not be \c NULL.
 * \param audio_rate      The audio rate -- must be positive.
 * \param buf_start       The start index of the buffer area to be processed.
 * \param buf_stop        The stop index of the buffer area to be processed.
 *
 * \return   A new buffer stop index that is within the range of
 *           [\a buf_start, \a buf_stop]. Values less than \a buf_stop
 *           indicate that permanent silence is reached and the voice should be
 *           deactivated after the current process cycle.
 */
int32_t Voice_state_common_ramp_release(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);
#endif


#endif // K_VOICE_STATE_COMMON_H


