

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


#ifndef AUDIO_H
#define AUDIO_H


#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

#include <kqt_Context.h>
#include <kqt_Mix_state.h>


#define AUDIO_ERROR_LENGTH (128)


typedef struct Audio
{
    char* name;
    bool active;
    char error[AUDIO_ERROR_LENGTH];
    bool pause;
    uint32_t nframes;
    uint32_t freq;
    kqt_Context* context;
    bool (*open)(struct Audio*);
    bool (*close)(struct Audio*);
    bool (*set_buffer_size)(struct Audio*, uint32_t nframes);
    bool (*set_freq)(struct Audio*, uint32_t freq);
    void (*destroy)(struct Audio*);
    pthread_cond_t state_cond;
    pthread_mutex_t state_mutex;
    kqt_Mix_state state;
} Audio;


/**
 * Creates a new Audio.
 *
 * \param name   The name of the driver -- must not be \c NULL.
 *
 * \return   The new driver if successful, or \c NULL if memory allocation
 *           failed.
 */
Audio* new_Audio(char* name);


/**
 * Gets the name of the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   The name.
 */
char* Audio_get_name(Audio* audio);


/**
 * Gets an error message from the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   The error message. This is never \c NULL. Empty string indicates
 *           that the previous operation succeeded.
 */
char* Audio_get_error(Audio* audio);


/**
 * Initialises the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param name      The name of the Audio -- must not be \c NULL.
 * \param open      The opening function -- must not be \c NULL.
 * \param close     The closing function -- must not be \c NULL.
 * \param destroy   The destructor of the Audio subclass -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_init(Audio* audio,
                char* name,
                bool (*open)(Audio*),
                bool (*close)(Audio*),
                void (*destroy)(Audio*));


/**
 * Sets the buffer size in the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param nframes   The new size in frames -- must be > \c 0.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_set_buffer_size(Audio* audio, uint32_t nframes);


/**
 * Sets the mixing frequency in the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 * \param freq    The mixing frequency -- must be > \c 0.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_set_freq(Audio* audio, uint32_t freq);


/**
 * Opens the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_open(Audio* audio);


/**
 * Closes the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_close(Audio* audio);


/**
 * Sets the Kunquat Context for the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param context   The Context.
 */
void Audio_set_context(Audio* audio, kqt_Context* context);


/**
 * Gets the mixing frequency of the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   The mixing frequency.
 */
uint32_t Audio_get_freq(Audio* audio);


/**
 * Gets the buffer size of the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   The buffer size in frames.
 */
uint32_t Audio_get_buffer_size(Audio* audio);


/**
 * Pauses/resumes audio processing in the Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 * \param pause   \c true to pause, \c false to resume.
 */
void Audio_pause(Audio* audio, bool pause);


/**
 * Gets an updated state from the Audio.
 *
 * This call may block the calling thread. It may only be called between
 * \a Audio_open and \a Audio_close calls.
 *
 * \param audio   The Audio -- must not be \c NULL and must be open.
 * \param state   The Mix state structure -- must not be \c NULL.
 *
 * \return   \c true if Mix state could be retrieved, otherwise \c false.
 */
bool Audio_get_state(Audio* audio, kqt_Mix_state* state);


/**
 * Sends a notification of state information change.
 *
 * This function does not set the Audio error message.
 *
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   \c 0 if successful, otherwise an error code from POSIX thread
 *           functions.
 */
int Audio_notify(Audio* audio);


/**
 * Sets an error message in the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param message   The error message format -- must not be \c NULL. This and
 *                  subsequent arguments follow the printf family conventions.
 */
void Audio_set_error(Audio* audio, char* message, ...);


/**
 * Destroys an existing Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 */
void del_Audio(Audio* audio);


#endif // AUDIO_H


