

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
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
 * int buffer_count = kqt_Handle_get_buffer_count(handle);
 * long buffer_size = kqt_Handle_get_buffer_size(handle);
 * long mixed = 0;
 * while ((mixed = kqt_Handle_mix(handle, buffer_size, 48000)) > 0)
 * {
 *     kqt_frame* buffers[KQT_BUFFERS_MAX] = { NULL };
 *     for (int i = 0; i < buffer_count; ++i)
 *     {
 *         buffers[i] = kqt_Handle_get_buffer(handle, i);
 *     }
 *     // convert (if necessary) and store the contents of the
 *     // buffers into the output buffers of the program,
 *     // then play the contents of the output buffers
 * }
 * \endcode
 */


/**
 * Mixes audio according to the state of the Kunquat Handle.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 * \param freq      The mixing frequency in frames per second
 *                  -- should be > \c 0. Typical values are
 *                  44100 ("CD quality") and 48000.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
long kqt_Handle_mix(kqt_Handle* handle, long nframes, long freq);


/**
 * Gets a mixing buffer from the Kunquat Handle.
 *
 * When called after a successful call of kqt_Handle_mix, this function
 * returns a portion of mixed audio of one output channel. The parameter
 * \a index determines the channel.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param index    The output channel number. \c 0 is the left
 *                 mixing buffer and \c 1 is the right one.
 *
 * \return   The buffer, or \c NULL if \a handle == \c NULL or \a index
 *           is out of range.
 *           The buffer contains sample values normalised to the range
 *           [-1.0, 1.0]. However, values beyond this range are possible
 *           and they indicate clipping.
 *           Note: Do not cache the returned value! The location of the buffer
 *           may change in memory, especially if the buffer size or the number
 *           of buffers is changed.
 */
kqt_frame* kqt_Handle_get_buffer(kqt_Handle* handle, int index);


/**
 * Sets the buffer size of the Kunquat Handle.
 *
 * This function is useful if the output buffer size changes in the calling
 * application. See kqt_new_Handle_r for detailed explanation of how the
 * buffer size is interpreted.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param size     The new buffer size -- should be > \c 0.
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
 * \return   The size of a buffer in frames.
 */
long kqt_Handle_get_buffer_size(kqt_Handle* handle);


/**
 * Gets the duration of the current Subsong in the Kunquat Handle in
 * nanoseconds.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
long long kqt_Handle_get_duration(kqt_Handle* handle);


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


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_PLAYER_H


