

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


typedef struct Audio
{
    bool active;
    uint32_t nframes;
    uint32_t freq;
    kqt_Context* context;
    void (*destroy)(struct Audio*);
    pthread_cond_t state_cond;
    pthread_mutex_t state_mutex;
    kqt_Mix_state state;
} Audio;


/**
 * Initialises the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param destroy   The destructor of the Audio subclass -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_init(Audio* audio, void (*destroy)(Audio*));


/**
 * Sets the kqt_Context for the Audio.
 *
 * \param audio     The Audio -- must not be \c NULL.
 * \param context   The kqt_Context -- must not be \c NULL.
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
 * \param audio   The Audio -- must not be \c NULL.
 *
 * \return   \c 0 if successful, otherwise an error code from POSIX thread
 *           functions.
 */
int Audio_notify(Audio* audio);


/**
 * Destroys an existing Audio.
 *
 * \param audio   The Audio -- must not be \c NULL.
 */
void del_Audio(Audio* audio);


#endif // AUDIO_H


