

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


#ifndef K_PROC_UTILS_H
#define K_PROC_UTILS_H


#include <decl.h>
#include <init/devices/Processor.h>
#include <player/devices/Device_state.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Create a default Processor state.
 *
 * \param device              The Processor associated with the state
 *                            -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be positive.
 * \param audio_buffer_size   The audio buffer size -- must not be negative.
 *
 * \return   The new Processor state if successful, or \c NULL if memory
 *           allocation failed.
 */
Proc_state* new_Proc_state_default(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);


/**
 * Process volume ramping at the start of a note.
 *
 * This function should be called by processor implementations (that need it)
 * before returning from their process function.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param buf_count    The number of output buffers -- must be > \c 0.
 * \param out_bufs     The signal output buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed.
 * \param buf_stop     The stop index of the buffer area to be processed.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Proc_ramp_attack(
        Voice_state* vstate,
        int buf_count,
        float* out_bufs[buf_count],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate);


#if 0
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
 * Retrieve raw input buffers from the Processor.
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
 * Retrieve raw output buffers from the Processor.
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
#endif


/**
 * A helper for conditional Work buffer access.
 *
 * If a voice feature is disabled, this class provides an interface to a
 * corresponding virtual buffer filled with neutral values.
 */
typedef struct Cond_work_buffer
{
    int32_t index_mask;
    float def_value;
    const float* wb_contents;
} Cond_work_buffer;


#define COND_WORK_BUFFER_AUTO &(Cond_work_buffer){ \
    .index_mask = 0, .def_value = 0, .wb_contents = NULL }


/**
 * Initialise a Conditional work buffer.
 *
 * \param cwb         The Conditional work buffer -- must not be \c NULL.
 * \param wb          The Work buffer, or \c NULL.
 * \param def_value   The default value.
 * \param enabled     \c true if \a wb should be used, otherwise \c false.
 *
 * \return   The parameter \a cwb.
 */
Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value, bool enabled);


/**
 * Get a value from the Conditional work buffer.
 *
 * \param cwb     The Conditional work buffer -- must not be \c NULL.
 * \param index   The index -- must be less than the Work buffer size.
 *
 * \return   The value at \a index.
 */
inline float Cond_work_buffer_get_value(const Cond_work_buffer* cwb, int32_t index)
{
    assert(cwb != NULL);
    return cwb->wb_contents[index & cwb->index_mask];
}


#endif // K_PROC_UTILS_H


