

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_COMMON_H
#define K_DSP_COMMON_H


#include <stdlib.h>
#include <stdbool.h>

#include <xassert.h>


#define DSP_get_raw_buffers(ds, type, port, buffers)      \
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
 * Retrieves raw input buffers from the DSP Device.
 *
 * This macro stops the calling function if the buffers do not exist.
 *
 * \param ds        The Device state -- must not be \c NULL.
 * \param num       The port number -- must be >= \c and
 *                  < \c KQT_DEVICE_PORTS_MAX.
 * \param buffers   The array where the buffers are stored -- must not
 *                  be \c NULL.
 */
#define DSP_get_raw_input(ds, port, buffers) \
    DSP_get_raw_buffers((ds), DEVICE_PORT_TYPE_RECEIVE, (port), (buffers))


/**
 * Retrieves raw output buffers from the DSP Device.
 *
 * This macro stops the calling function if the buffers do not exist.
 *
 * \param ds        The Device state -- must not be \c NULL.
 * \param num       The port number -- must be >= \c and
 *                  < \c KQT_DEVICE_PORTS_MAX.
 * \param buffers   The array where the buffers are stored -- must not
 *                  be \c NULL.
 */
#define DSP_get_raw_output(ds, port, buffers) \
    DSP_get_raw_buffers((ds), DEVICE_PORT_TYPE_SEND, (port), (buffers))


#endif // K_DSP_COMMON_H


