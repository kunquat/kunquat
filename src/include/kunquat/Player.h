

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


#ifndef KQT_PLAYER_H
#define KQT_PLAYER_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>
#include <kunquat/limits.h>


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
 * long buffer_size = kqt_Handle_get_audio_buffer_size(handle);
 * kqt_Handle_play(handle);
 * while (!kqt_Handle_has_stopped(handle))
 * {
 *     long frames_available = kqt_Handle_get_frames_available(handle);
 *     float* buffers[] =
 *     {
 *         kqt_Handle_get_audio(handle, 0), // left
 *         kqt_Handle_get_audio(handle, 1), // right
 *     };
 *
 *     // Convert (if necessary) and store the contents of the
 *     // buffers into the output buffers of the program,
 *     // then play the contents of the output buffers.
 *
 *     kqt_Handle_play(handle);
 * }
 * \endcode
 */


/**
 * Play music according to the state of the Kunquat Handle.
 *
 * If this function is called after the end of the composition is reached,
 * more audio is produced as if the composition were paused indefinitely.
 *
 * \param handle    The Handle -- should be valid.
 * \param nframes   The number of frames to be rendered -- should be > \c 0.
 *
 * \return   \c 1 if successful, or \c 0 if an error occurred.
 */
int kqt_Handle_play(kqt_Handle handle, long nframes);


/**
 * Find out if playback has stopped in the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   \c 1 if playback has stopped, \c 0 if playback has not stopped,
 *           or \c -1 if an error occurred.
 */
int kqt_Handle_has_stopped(kqt_Handle handle);


/**
 * Get the amount of audio data available in internal audio buffers.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The number of frames available in each buffer, or \c -1 if an
 *           error occurred. A return value of \c 0 does not imply end of
 *           playback or error; a subsequent call of kqt_Handle_play may
 *           produce more audio data.
 */
long kqt_Handle_get_frames_available(kqt_Handle handle);


/**
 * Get an audio buffer from the Kunquat Handle.
 *
 * When called after a successful call of kqt_Handle_play, this function
 * returns a portion of rendered audio of one output channel. The parameter
 * \a index specifies the output channel.
 *
 * \param handle   The Handle -- should be valid.
 * \param index    The output channel number. \c 0 is the left
 *                 mixing buffer and \c 1 is the right one.
 *
 * \return   The buffer, or \c NULL if \a handle is not valid or \a index
 *           is out of range.
 *           The buffer contains sample values normalised to the range
 *           [-1.0, 1.0]. However, values beyond this range are possible
 *           and they indicate clipping.
 *           Note: Do not cache the returned value! The location of the buffer
 *           may change in memory.
 */
const float* kqt_Handle_get_audio(kqt_Handle handle, int index);


/**
 * Set the audio rate of the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 * \param rate     The audio rate in frames per second -- should be > \c 0.
 *                 Typical values include 48000 (the default) and 44100 ("CD
 *                 quality").
 *
 * \return   \c 1 if successful, or \c 0 if memory allocation failed. Memory
 *           allocation failure is possible if the composition uses features
 *           that allocate buffers based on the audio rate.
 */
int kqt_Handle_set_audio_rate(kqt_Handle handle, long rate);


/**
 * Get the current audio rate used by the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The current audio rate, or \c 0 if \a handle is invalid.
 */
long kqt_Handle_get_audio_rate(kqt_Handle handle);


/**
 * Set the number of threads used in audio rendering by the Kunquat Handle.
 *
 * NOTE: If libkunquat is built without thread support, this function will have
 *       no effect.
 *
 * \param handle   The Handle -- should be valid.
 * \param count    The number of threads -- should be >= \c 1 and
 *                 <= \c KQT_THREADS_MAX.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_set_thread_count(kqt_Handle handle, int count);


/**
 * Get the number of threads used in audio rendering by the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The number of threads used.
 */
int kqt_Handle_get_thread_count(kqt_Handle handle);


/**
 * Set the audio buffer size of the Kunquat Handle.
 *
 * The buffer size determines the maximum amount of audio data that can
 * be rendered in one call. The buffer size is given as the number of amplitude
 * values (i.e. \a frames) for one output channel. In a typical case, the
 * calling application should set this value based on the size of its own
 * output buffers: if the application uses buffers with \a n amplitude values
 * for one output channel (e.g. in 16-bit stereo, this takes \a n * \c 4 bytes
 * in total), it should call kqt_Handle_set_buffer_size with a buffer size of
 * \a n.
 *
 * \param handle   The Handle -- should be valid.
 * \param size     The new buffer size -- should be > \c 0 and
 *                 <= \c KQT_BUFFER_SIZE_MAX. The upper limit is a safety
 *                 measure; typically, implementations use a buffer size of
 *                 no more than a couple of thousand frames.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 *           Note: If memory allocation fails, playback is still possible but
 *           only with min{old_size, new_size} frames at a time. However, it
 *           may be a good idea to just give up in this case.
 */
int kqt_Handle_set_audio_buffer_size(kqt_Handle handle, long size);


/**
 * Get the audio buffer size of the Kunquat Handle.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The size of a buffer in frames, or \c 0 if \a handle is invalid.
 */
long kqt_Handle_get_audio_buffer_size(kqt_Handle handle);


/**
 * Estimate the duration of a track in the Kunquat Handle.
 *
 * This function will not calculate the length of a track further
 * than KQT_CALC_DURATION_MAX nanoseconds.
 *
 * \param handle   The Handle -- should be valid.
 * \param track    The track number -- should be >= \c -1 and
 *                 < \c KQT_TRACKS_MAX (\c -1 denotes all tracks).
 *
 * \return   The length in nanoseconds, or KQT_CALC_DURATION_MAX if the length
 *           is KQT_CALC_DURATION_MAX nanoseconds or longer, or \c -1 if
 *           failed.
 */
long long kqt_Handle_get_duration(kqt_Handle handle, int track);


/**
 * Set the position to be played.
 *
 * Any notes that were being mixed will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param handle        The Handle -- should be valid.
 * \param track         The track number -- should be >= \c -1 and
 *                      < \c KQT_TRACKS_MAX (\c -1 denotes all tracks).
 * \param nanoseconds   The number of nanoseconds from the beginning --
 *                      should not be negative.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Handle_set_position(kqt_Handle handle, int track, long long nanoseconds);


/**
 * Get the current position in nanoseconds.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   The amount of nanoseconds mixed since the start of mixing.
 */
long long kqt_Handle_get_position(kqt_Handle handle);


/**
 * Fire an event.
 *
 * \param handle    The Handle -- should be valid.
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
int kqt_Handle_fire_event(kqt_Handle handle, int channel, const char* event);


/**
 * Return a JSON list of events.
 *
 * The returned list of events is not necessarily exhaustive; subsequent
 * calls may return more events.
 *
 * \param handle   The Handle -- should be valid.
 *
 * \return   A JSON list of events if successful, or \c NULL if an error
 *           occurred. An empty list indicates that all events have been
 *           returned.
 */
const char* kqt_Handle_receive_events(kqt_Handle handle);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_PLAYER_H


