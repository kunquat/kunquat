

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef KQT_CONTEXT_H
#define KQT_CONTEXT_H


#include <stdint.h>

#include <kunquat/frame.h>
#include <kunquat/Mix_state.h>


#define MAX_VOICES (1024)


typedef struct kqt_Context kqt_Context;


/**
 * Creates a new Kunquat Context.
 *
 * \param buf_count          The number of buffers used for mixing. Currently,
 *                           this can be 1 (mono) or 2 (stereo).
 * \param buf_size           The size of the mixing buffer.
 * \param voice_count        The number of Voices used for mixing.
 * \param event_queue_size   The size of the Event queue for each Column.
 *
 * \return   The new Kunquat Context if successful, or \c NULL if memory
 *           allocation failed.
 */
kqt_Context* kqt_new_Context(int buf_count,
                             uint32_t buf_size,
                             uint16_t voice_count,
                             uint8_t event_queue_size);


/**
 * Gets error information from the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The error message. This is an empty string if no error has
 *           occurred.
 */
char* kqt_Context_get_error(kqt_Context* context);


/**
 * Loads contents of a Kunquat file or directory into the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param path      The path to the Kunquat composition file or directory
 *                  -- should not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool kqt_Context_load(kqt_Context* context, char* path);


/**
 * Gets the length of a Subsong in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c 0 and
 *                  < \c SUBSONGS_MAX.
 *
 * \return   The length of the Subsong, or \c -1 if arguments were invalid.
 */
int kqt_Context_get_subsong_length(kqt_Context* context, int subsong);


/**
 * Gets the length of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
uint64_t kqt_Context_get_length_ns(kqt_Context* context);


/**
 * Gets the number of mixing buffers in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of buffers, or \c 0 if \a context == \c NULL.
 */
int kqt_Context_get_buffer_count(kqt_Context* context);


/**
 * Gets the mixing buffers in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The buffers, or \c NULL if \a context == \c NULL.
 */
kqt_frame** kqt_Context_get_buffers(kqt_Context* context);


/**
 * Resizes the buffers in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param size      The new buffer size -- should be > \c 0.
 *
 * \return   \c true if successful, otherwise \c false.
 *           Note: If memory allocation fails, mixing is still possible but
 *           only with min{old_size, new_size} frames at a time. However, it
 *           may be a good idea to just give up in this case.
 */
bool kqt_Context_set_buffer_size(kqt_Context* context, uint32_t size);


/**
 * Sets the position to be played in nanoseconds.
 *
 * Any notes that were being played will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param context       The Context -- should not be \c NULL.
 * \param nanoseconds   The number of nanoseconds from the beginning --
 *                      should not be negative.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool kqt_Context_set_position_ns(kqt_Context* context, long long nanoseconds);


/**
 * Gets the current position in nanoseconds.
 *
 * \param context       The Context -- should not be \c NULL.
 *
 * \return   The amount of nanoseconds mixed since the start of mixing.
 */
long long kqt_Context_get_position_ns(kqt_Context* context);


/**
 * Sets the position to be played.
 *
 * Any notes that were being played will be cut off immediately.
 * Notes that start playing before the given position will not be played.
 *
 * \param context    The Context -- should not be \c NULL.
 * \param position   The new position -- should not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool kqt_Context_set_position(kqt_Context* context, char* position);


/**
 * Does mixing according to the state of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param nframes   The number of frames to be mixed.
 * \param freq      The mixing frequency -- should be > \c 0.
 *
 * \return   The number of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t kqt_Context_mix(kqt_Context* context, uint32_t nframes, uint32_t freq);


/**
 * Tells whether mixing of the Kunquat Context has reached the end.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   \c true if the end has been reached, otherwise \c false.
 */
bool kqt_Context_end_reached(kqt_Context* context);


/**
 * Gets the total number of frames mixed after the last position change.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of frames.
 */
uint64_t kqt_Context_get_frames_mixed(kqt_Context* context);


/**
 * Gets the current position of the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The position description, or \c NULL if \a context is invalid.
 */
char* kqt_Context_get_position(kqt_Context* context);


/**
 * Gets the current tempo in the Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The current tempo.
 */
double kqt_Context_get_tempo(kqt_Context* context);


/**
 * Gets the maximum number of simultaneous Voices used.
 *
 * \param context   The Context -- should not be \c NULL.
 *
 * \return   The number of Voices.
 */
int kqt_Context_get_voice_count(kqt_Context* context);


/**
 * Gets the minimum amplitude value encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The minimum amplitude value, or \c INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Context_get_min_amplitude(kqt_Context* context, int buffer);


/**
 * Gets the maximum amplitude value encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The maximum amplitude value, or \c -INFINITY if nothing has been
 *           mixed into the buffer.
 */
double kqt_Context_get_max_amplitude(kqt_Context* context, int buffer);


/**
 * Gets the number of clipped frames encountered.
 *
 * \param context   The Context -- should not be \c NULL.
 * \param buffer    The buffer index (0 for left, 1 for right).
 *
 * \return   The number of clipped frames.
 */
long kqt_Context_get_clipped(kqt_Context* context, int buffer);


/**
 * Resets mixing state statistics.
 *
 * The values that will be reset are number of Voices used, peak amplitude
 * values and counts of clipped frames. Thus, it doesn't affect the playback.
 *
 * \param context   The Context -- should not be \c NULL.
 */
void kqt_Context_reset_stats(kqt_Context* context);


/**
 * Gets playback statistics from the Kunquat Context.
 *
 * This is a convenience function that retrieves the information provided by
 * the following functions:
 *
 *    kqt_Context_end_reached
 *    kqt_Context_get_position
 *    kqt_Context_get_tempo
 *    kqt_Context_get_voice_count
 *    kqt_Context_get_min_amplitude
 *    kqt_Context_get_max_amplitude
 *    kqt_Context_get_clipped
 *    
 * It also calls kqt_Context_reset_stats.
 *
 * \param context     The Context -- should not be \c NULL.
 * \param mix_state   The Mix state where the statistics shall be written.
 */
void kqt_Context_get_state(kqt_Context* context, kqt_Mix_state* mix_state);


/**
 * Destroys an existing Kunquat Context.
 *
 * \param context   The Context -- should not be \c NULL.
 */
void kqt_del_Context(kqt_Context* context);


#endif // KQT_CONTEXT_H


