

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


#if 0
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
#endif


#endif // K_VOICE_STATE_COMMON_H


