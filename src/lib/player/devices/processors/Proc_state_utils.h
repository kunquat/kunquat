

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


#ifndef KQT_PROC_STATE_UTILS_H
#define KQT_PROC_STATE_UTILS_H


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
 * Retrieve mixed audio input buffers.
 *
 * \param proc_ts      The Device thread state -- must not be \c NULL.
 * \param port_start   The first port index -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 * \param port_stop    The index after the last port index -- must be
 *                     >= \c port_start and <= \c KQT_DEVICE_PORTS_MAX.
 * \param in_bufs      The destination buffer array -- must not be \c NULL and
 *                     must have space for \a port_stop - \a port_start buffers.
 *                     Any of the stored pointers may be \c NULL.
 */
void Proc_state_get_mixed_audio_in_buffers(
        Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* in_bufs[]);


/**
 * Retrieve mixed audio output buffers.
 *
 * \param proc_ts      The Device thread state -- must not be \c NULL.
 * \param port_start   The first port index -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 * \param port_stop    The index after the last port index -- must be
 *                     >= \c port_start and <= \c KQT_DEVICE_PORTS_MAX.
 * \param out_bufs     The destination buffer array -- must not be \c NULL and
 *                     must have space for \a port_stop - \a port_start buffers.
 *                     Any of the stored pointers may be \c NULL.
 */
void Proc_state_get_mixed_audio_out_buffers(
        Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[]);


/**
 * Retrieve voice audio input buffers.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param proc_ts      The Device thread state -- must not be \c NULL.
 * \param port_start   The first port index -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 * \param port_stop    The index after the last port index -- must be
 *                     >= \c port_start and <= \c KQT_DEVICE_PORTS_MAX.
 * \param in_bufs      The destination buffer array -- must not be \c NULL and
 *                     must have space for \a port_stop - \a port_start buffers.
 *                     Any of the stored pointers may be \c NULL.
 */
void Proc_state_get_voice_audio_in_buffers(
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* in_bufs[]);


/**
 * Retrieve voice audio output buffers.
 *
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param proc_ts      The Device thread state -- must not be \c NULL.
 * \param port_start   The first port index -- must be >= \c 0 and
 *                     < \c KQT_DEVICE_PORTS_MAX.
 * \param port_stop    The index after the last port index -- must be
 *                     >= \c port_start and <= \c KQT_DEVICE_PORTS_MAX.
 * \param out_bufs     The destination buffer array -- must not be \c NULL and
 *                     must have space for \a port_stop - \a port_start buffers.
 *                     Any of the stored pointers may be \c NULL.
 */
void Proc_state_get_voice_audio_out_buffers(
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        int32_t port_start,
        int32_t port_stop,
        float* out_bufs[]);


/**
 * Process volume ramping at the start of a note.
 *
 * This function should be called by processor implementations (that need it)
 * before returning from their process function.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param buf_count    The number of output buffers -- must be > \c 0.
 * \param out_bufs     The signal output buffers -- must not be \c NULL and
 *                     must contain at least \a buf_count buffers.
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


/**
 * Convert pitch values to frequencies.
 *
 * NOTE: This function assumes that the index range defined by \a buf_start
 *       and \a buf_stop is the full range used in the current rendering cycle.
 *       If that is not the case, the const start value of \a freqs may be
 *       incorrect.
 *
 * \param freqs       The destination buffer -- must not be \c NULL.
 * \param pitches     The pitch buffer -- must not be \c NULL. This buffer may
 *                    be the same as \a freqs.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Proc_fill_freq_buffer(
        Work_buffer* freqs,
        Work_buffer* pitches,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Convert decibel values to scales.
 *
 * NOTE: This function assumes that the index range defined by \a buf_start
 *       and \a buf_stop is the full range used in the current rendering cycle.
 *       If that is not the case, the const start value of \a scales may be
 *       incorrect.
 *
 * \param scales      The destination buffer -- must not be \c NULL.
 * \param dBs         The decibel buffer -- must not be \c NULL. This buffer
 *                    may be the same as \a scales.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Proc_fill_scale_buffer(
        Work_buffer* scales,
        Work_buffer* dBs,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Clamp pitch buffer contents to a safe range.
 *
 * \param pitches     The pitch buffer -- must not be \c NULL.
 * \param buf_start   The start index of the buffer area to be processed.
 * \param buf_stop    The stop index of the buffer area to be processed.
 */
void Proc_clamp_pitch_values(Work_buffer* pitches, int32_t buf_start, int32_t buf_stop);


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
 *
 * \return   The parameter \a cwb.
 */
Cond_work_buffer* Cond_work_buffer_init(
        Cond_work_buffer* cwb, const Work_buffer* wb, float def_value);


/**
 * Get a value from the Conditional work buffer.
 *
 * \param cwb     The Conditional work buffer -- must not be \c NULL.
 * \param index   The index -- must be less than the Work buffer size.
 *
 * \return   The value at \a index.
 */
static inline float Cond_work_buffer_get_value(
        const Cond_work_buffer* cwb, int32_t index)
{
    rassert(cwb != NULL);
    return cwb->wb_contents[index & cwb->index_mask];
}


#endif // KQT_PROC_STATE_UTILS_H


