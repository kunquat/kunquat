

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_COMMON_H
#define K_PROC_COMMON_H


#include <Audio_buffer.h>
#include <Decl.h>
#include <devices/Processor.h>
#include <kunquat/limits.h>
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
 * \param proc        The Processor -- must not be \c NULL.
 * \param vstate      The Voice state -- must not be \c NULL.
 * \param wbs         The Work buffers -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Proc_common_handle_pitch(
        const Processor* proc,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process force.
 *
 * This function fills WORK_BUFFER_ACTUAL_FORCES with up-to-date values in the
 * specified buffer area. It must be called after \a Proc_common_handle_pitch.
 *
 * \param proc          The Processor -- must not be \c NULL.
 * \param au_state      The Audio unit state -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
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
int32_t Proc_common_handle_force(
        const Processor* proc,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process lowpass filter.
 *
 * This function fills WORK_BUFFER_ACTUAL_LOWPASSES with up-to-date values in
 * the specified buffer area. It must be called after
 * \a Proc_common_handle_force and the process function of the processor
 * implementation.
 *
 * \param proc            The Processor -- must not be \c NULL.
 * \param au_state        The Audio unit state -- must not be \c NULL.
 * \param vstate          The Voice state -- must not be \c NULL.
 * \param wbs             The Work buffers -- must not be \c NULL.
 * \param voice_out_buf   The audio output buffer -- must not be \c NULL.
 * \param ab_count        The number of audio buffers used -- must be \c 1 or
 *                        \c 2. If \c 1, only the left channel will be updated.
 * \param audio_rate      The audio rate -- must be positive.
 * \param buf_start       The start index of the buffer area to be processed.
 * \param buf_stop        The stop index of the buffer area to be processed.
 */
void Proc_common_handle_filter(
        const Processor* proc,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        Audio_buffer* voice_out_buf,
        int ab_count,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process volume ramping at the end of a note.
 *
 * This function should be called after the process function of the processor
 * implementation.
 *
 * \param proc            The Processor -- must not be \c NULL.
 * \param au_state        The Audio unit state -- must not be \c NULL.
 * \param vstate          The Voice state -- must not be \c NULL.
 * \param wbs             The Work buffers -- must not be \c NULL.
 * \param voice_out_buf   The audio output buffer -- must not be \c NULL.
 * \param ab_count        The number of audio buffers used -- must be \c 1 or
 *                        \c 2. If \c 1, only the left channel will be updated.
 * \param audio_rate      The audio rate -- must be positive.
 * \param buf_start       The start index of the buffer area to be processed.
 * \param buf_stop        The stop index of the buffer area to be processed.
 *
 * \return   A new buffer stop index that is within the range of
 *           [\a buf_start, \a buf_stop]. Values less than \a buf_stop
 *           indicate that permanent silence is reached and the voice should be
 *           deactivated after the current process cycle.
 */
int32_t Proc_common_ramp_release(
        const Processor* proc,
        const Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        Audio_buffer* voice_out_buf,
        int ab_count,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Process panning.
 *
 * This function should be called after the process function of the processor
 * implementation.
 *
 * \param proc            The Processor -- must not be \c NULL.
 * \param vstate          The Voice state -- must not be \c NULL.
 * \param wbs             The Work buffers -- must not be \c NULL.
 * \param voice_out_buf   The audio output buffer -- must not be \c NULL.
 * \param buf_start       The start index of the buffer area to be processed.
 * \param buf_stop        The stop index of the buffer area to be processed.
 */
void Proc_common_handle_panning(
        const Processor* proc,
        Voice_state* vstate,
        const Work_buffers* wbs,
        Audio_buffer* voice_out_buf,
        int32_t buf_start,
        int32_t buf_stop);


#endif // K_PROC_COMMON_H


