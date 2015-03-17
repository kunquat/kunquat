

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GEN_UTILS_H
#define K_GEN_UTILS_H


#include <stdint.h>
#include <stdlib.h>

#include <devices/Generator.h>
#include <player/Voice_state.h>
#include <player/Work_buffers.h>


/**
 * Process volume ramping at the start of a note.
 *
 * This function modifies WORK_BUFFER_AUDIO_* and should be called by generator
 * implementations (that need it) before returning from their process
 * function.
 *
 * \param gen          The Generator -- must not be \c NULL.
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param ab_count     The number of audio buffers used -- must be \c 1 or
 *                     \c 2. If \c 1, WORK_BUFFER_AUDIO_L will be updated.
 * \param audio_rate   The audio rate -- must be positive.
 * \param buf_start    The start index of the buffer area to be processed.
 * \param buf_stop     The stop index of the buffer area to be processed.
 */
void Generator_common_ramp_attack(
        const Generator* gen,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int ab_count,
        uint32_t audio_rate,
        int32_t buf_start,
        int32_t buf_stop);


#define get_raw_buffers(ds, type, port, buffers)          \
    if (true)                                             \
    {                                                     \
        assert((buffers) != NULL);                        \
        Audio_buffer* in = Device_state_get_audio_buffer( \
                (ds), (type), (port));                    \
        if (in == NULL)                                   \
            return;                                       \
                                                          \
        (buffers)[0] = Audio_buffer_get_buffer(in, 0);    \
        (buffers)[1] = Audio_buffer_get_buffer(in, 1);    \
    } else (void)0


/**
 * Retrieve raw input buffers from the Generator.
 *
 * This macro stops the calling function if the buffers do not exist.
 *
 * \param ds        The Device state -- must not be \c NULL.
 * \param num       The port number -- must be >= \c and
 *                  < \c KQT_DEVICE_PORTS_MAX.
 * \param buffers   The array where the buffers are stored -- must not
 *                  be \c NULL.
 */
#define get_raw_input(ds, port, buffers) \
    get_raw_buffers((ds), DEVICE_PORT_TYPE_RECEIVE, (port), (buffers))


/**
 * Retrieve raw output buffers from the Generator.
 *
 * This macro stops the calling function if the buffers do not exist.
 *
 * \param ds        The Device state -- must not be \c NULL.
 * \param num       The port number -- must be >= \c and
 *                  < \c KQT_DEVICE_PORTS_MAX.
 * \param buffers   The array where the buffers are stored -- must not
 *                  be \c NULL.
 */
#define get_raw_output(ds, port, buffers) \
    get_raw_buffers((ds), DEVICE_PORT_TYPE_SEND, (port), (buffers))


#endif // K_GEN_UTILS_H


