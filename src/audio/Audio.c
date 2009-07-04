

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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>

#include <pthread.h>

#include <Audio.h>
#include <kqt_Context.h>
#include <kqt_Mix_state.h>

#include <xmemory.h>


bool Audio_init(Audio* audio,
                bool (*open)(Audio*),
                bool (*close)(Audio*),
                void (*destroy)(Audio*))
{
    assert(audio != NULL);
    assert(open != NULL);
    assert(close != NULL);
    assert(destroy != NULL);
    audio->active = false;
    audio->nframes = 0;
    audio->freq = 0;
    audio->context = NULL;
    kqt_Mix_state_init(&audio->state);
    audio->open = open;
    audio->close = close;
    audio->destroy = destroy;
    if (pthread_cond_init(&audio->state_cond, NULL) < 0)
    {
        return false;
    }
    if (pthread_mutex_init(&audio->state_mutex, NULL) < 0)
    {
        pthread_cond_destroy(&audio->state_cond);
        return false;
    }
    return true;
}


bool Audio_open(Audio* audio)
{
    assert(audio != NULL);
    assert(audio->open != NULL);
    return audio->open(audio);
}


bool Audio_close(Audio* audio)
{
    assert(audio != NULL);
    assert(audio->close != NULL);
    return audio->close(audio);
}


void Audio_set_context(Audio* audio, kqt_Context* context)
{
    assert(audio != NULL);
    audio->context = context;
    return;
}


uint32_t Audio_get_freq(Audio* audio)
{
    assert(audio != NULL);
    return audio->freq;
}


uint32_t Audio_get_buffer_size(Audio* audio)
{
    assert(audio != NULL);
    return audio->nframes;
}


bool Audio_get_state(Audio* audio, kqt_Mix_state* state)
{
    assert(audio != NULL);
    assert(state != NULL);
    assert(audio->active);
    if (pthread_mutex_lock(&audio->state_mutex) != 0)
    {
        return false;
    }
    if (pthread_cond_wait(&audio->state_cond, &audio->state_mutex) != 0)
    {
        pthread_mutex_unlock(&audio->state_mutex);
        return false;
    }
    kqt_Mix_state_copy(state, &audio->state);
    pthread_mutex_unlock(&audio->state_mutex);
    return true;
}


void del_Audio(Audio* audio)
{
    assert(audio != NULL);
    assert(audio->destroy != NULL);
    if (audio->active)
    {
        audio->close(audio);
    }
    while (pthread_cond_destroy(&audio->state_cond))
    {
        sleep(1);
    }
    while (pthread_mutex_destroy(&audio->state_mutex))
    {
        sleep(1);
    }
    audio->destroy(audio);
    return;
}


int Audio_notify(Audio* audio)
{
    assert(audio != NULL);
    int err = pthread_mutex_trylock(&audio->state_mutex);
    if (err == 0)
    {
        if (audio->context != NULL)
        {
            kqt_Context_get_state(audio->context, &audio->state);
        }
        else
        {
            audio->state.playing = false;
        }
        err = pthread_cond_signal(&audio->state_cond);
        err = pthread_mutex_unlock(&audio->state_mutex);
    }
    return err;
}


