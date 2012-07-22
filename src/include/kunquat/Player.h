

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PLAYER_H
#define KQT_PLAYER_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>


/**
 * \defgroup Player Playing Kunquat compositions
 *
 * \{
 *
 * \brief
 * This module describes a simple API for applications that use libkunquat
 * for playing Kunquat compositions.
 *
 * After the Kunquat Handle has been created, the user can get the audio to
 * be played in short sections. First the user mixes a short section of music
 * into internal buffers and then transfers the necessary information from the
 * internal buffers into its own output buffers. This process is repeated as
 * many times as needed. The basic playback cycle might look like this:
 *
 * \code
 * long buffer_size = kqt_Handle_get_buffer_size(handle);
 * long mixed = 0;
 * while ((mixed = kqt_Handle_mix(handle, buffer_size)) > 0)
 * {
 *     float* buffers[KQT_BUFFERS_MAX] = { NULL };
 *     buffers[0] = kqt_Handle_get_buffer(handle, 0); // left
 *     buffers[1] = kqt_Handle_get_buffer(handle, 1); // right
 *     // Convert (if necessary) and store the contents of the
 *     // buffers into the output buffers of the program,
 *     // then play the contents of the output buffers.
 *     // The number of frames per buffer is stored in the variable mixed.
 * }
 * \endcode
 */


/**
 * Mixes audio according to the state of the Kunquat Handle.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param nframes   The number of frames to be mixed -- should be > \c 0.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes and <= kqt_Handle_get_buffer_size(handle).
 */
long kqt_Handle_mix(kqt_Handle* handle, long nframes);


/**
 * Gets a mixing buffer from the Kunquat Handle.
 *
 * When called after a successful call of kqt_Handle_mix, this function
 * returns a portion of mixed audio of one output channel. The parameter
 * \a index determines the output channel.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param index    The output channel number. \c 0 is the left
 *                 mixing buffer and \c 1 is the right one.
 *
 * \return   The buffer, or \c NULL if \a handle is not valid or \a index
 *           is out of range.
 *           The buffer contains sample values normalised to the range
 *           [-1.0, 1.0]. However, values beyond this range are possible
 *           and they indicate clipping.
 *           Note: Do not cache the returned value! The location of the buffer
 *           may change in memory, especially if the buffer size is changed.
 */
float* kqt_Handle_get_buffer(kqt_Handle* handle, int index);


/**
 * Sets the mixing rate of the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param rate     The mixing rate in frames per second -- should be > \c 0.
 *                 Typical values include 44100 ("CD quality") and 48000 (the
 *                 default).
 *
 * \return   \c 1 if successful, or \c 0 if memory allocation failed. Memory
 *           allocation failure is possible if the composition uses features
 *           that allocate buffers based on the mixing rate.
 */
int kqt_Handle_set_mixing_rate(kqt_Handle* handle, long rate);


/**
 * Gets the current mixing rate used by the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The current mixing rate, or \c 0 if \a handle is invalid.
 */
long kqt_Handle_get_mixing_rate(kqt_Handle* handle);


/**
 * Sets the buffer size of the Kunquat Handle.
 *
 * The buffer size determines the maximum amount of audio data that can
 * be mixed at one time. The buffer size is given as the number of amplitude
 * values (called \a frames) for one output channel. In a typical case, the
 * calling application should set this value based on the size of its own
 * output buffers: if the application uses buffers with \a n amplitude values
 * for one output channel (e.g. in 16-bit stereo, this takes \a n * \c 4 bytes
 * in total), it should call kqt_new_Handle with a buffer size of \a n.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param size     The new buffer size -- should be > \c 0 and
 *                 <= \c KQT_BUFFER_SIZE_MAX. The upper limit is a safety
 *                 measure -- typically, implementations use a buffer size of
 *                 no more than a couple of thousand frames.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 *           Note: If memory allocation fails, mixing is still possible but
 *           only with min{old_size, new_size} frames at a time. However, it
 *           may be a good idea to just give up in this case.
 */
int kqt_Handle_set_buffer_size(kqt_Handle* handle, long size);


/**
 * Gets the buffer size of the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The size of a buffer in frames, or \c 0 if \a handle is invalid.
 */
long kqt_Handle_get_buffer_size(kqt_Handle* handle);


/**
 * Estimates the duration of a Subsong in the Kunquat Handle.
 *
 * This function will not calculate the length of a Subsong further
 * than 30 days.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c -1 and
 *                  < \c KQT_SUBSONGS_MAX (\c -1 indicates all Subsongs).
 *
 * \return   The length in nanoseconds, or KQT_MAX_CALC_DURATION if the
 *           length is 30 days or longer, or \c -1 if failed.
 */
long long kqt_Handle_get_duration(kqt_Handle* handle, int subsong);


/**
 * Sets the position to be played.
 *
 * Any notes that were being mixed will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param handle        The Handle -- should not be \c NULL.
 * \param subsong       The Subsong number -- should be >= \c -1 and
 *                      < \c KQT_SUBSONGS_MAX (\c -1 indicates all Subsongs).
 * \param nanoseconds   The number of nanoseconds from the beginning --
 *                      should not be negative.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_set_position(kqt_Handle* handle, int subsong, long long nanoseconds);


/**
 * Gets the current position in nanoseconds.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The amount of nanoseconds mixed since the start of mixing.
 */
long long kqt_Handle_get_position(kqt_Handle* handle);


/**
 * Fires an event.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param channel   The channel where the event takes place -- should be
 *                  >= \c 0 and < \c KQT_COLUMNS_MAX.
 * \param event     The event description in JSON format -- should not be
 *                  \c NULL. The description is a pair (list with two
 *                  elements) with the event name as the first element and its
 *                  argument expression as the second element. The expression
 *                  should be null for events that do not support an argument
 *                  (e.g. ["cn-", null]).
 *
 * \return   \c 1 if the event was successfully fired, otherwise \c 0.
 */
int kqt_Handle_fire(kqt_Handle* handle, int channel, char* event);


/**
 * Receives an event.
 *
 * This function only returns events of types that are explicitly
 * requested through the ">receive" event.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param dest     The memory location where the result shall be stored
 *                 -- should not be \c NULL. Upon successful completion,
 *                 this memory location contains the received event
 *                 description as a JSON string.
 * \param size     The size of the memory area pointed to by \a dest --
 *                 should be positive. A size of at least 65 bytes is
 *                 recommended. JSON strings longer than \a size - 1
 *                 bytes are truncated and thus may be invalid.
 *
 * \return   \c 1 if an event was successfully retrieved, \c 0 if the
 *           event buffer is empty or an error occurred.
 */
int kqt_Handle_receive(kqt_Handle* handle, char* dest, int size);


/**
 * Receives an event specific to tracker integration.
 *
 * Currently, this function returns environment variable setter events.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param dest     The memory location where the result shall be stored
 *                 -- should not be \c NULL. Upon successful completion,
 *                 this memory location contains the received event
 *                 description as a JSON string.
 * \param size     The size of the memory area pointed to by \a dest --
 *                 should be positive. A size of at least 65 bytes is
 *                 recommended. JSON strings longer than \a size - 1
 *                 bytes are truncated and thus may be invalid.
 *
 * \return   \c 1 if an event was successfully retrieved, \c 0 if the
 *           event buffer is empty or an error occurred.
 */
int kqt_Handle_treceive(kqt_Handle* handle, char* dest, int size);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_PLAYER_H


