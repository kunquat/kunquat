

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <devices/Generator.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <player/Ins_state.h>
#include <player/Voice_state.h>


/**
 * Get the output buffers.
 */
#define Generator_common_get_buffers(gs, vstate, mixed, bufs) \
    if (true)                                                 \
    {                                                         \
        Audio_buffer* buffer = Device_state_get_audio_buffer( \
                &(gs)->parent, DEVICE_PORT_TYPE_SEND, 0);     \
        if (buffer == NULL)                                   \
        {                                                     \
            (vstate)->active = false;                         \
            return (mixed);                                   \
        }                                                     \
        bufs[0] = Audio_buffer_get_buffer(buffer, 0);         \
        bufs[1] = Audio_buffer_get_buffer(buffer, 1);         \
    } else (void)0


/**
 * Handle pitch.
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param vstate   The Voice state -- must not be \c NULL.
 */
void Generator_common_handle_pitch(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t nframes,
        int32_t offset);


/**
 * Handle force.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
int32_t Generator_common_handle_force(
        const Generator* gen,
        Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t freq,
        int32_t nframes,
        int32_t offset);


/**
 * Handle filter.
 *
 * This should be called after force handling.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_handle_filter(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset);


/**
 * Ramp volume for note start and end.
 *
 * This should be called after force handling if needed (not all Generators
 * need this).
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_ramp_attack(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset);


int32_t Generator_common_ramp_release(
        const Generator* gen,
        const Ins_state* ins_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t freq,
        int32_t nframes,
        int32_t offset);


/**
 * Handle panning.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param vstate        The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 */
void Generator_common_handle_panning(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t nframes,
        int32_t offset);


