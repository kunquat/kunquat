

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
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <unistd.h>

#include <sys/time.h>
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
    memset(audio->error, '\0', AUDIO_ERROR_LENGTH);
    audio->pause = false;
    audio->nframes = 0;
    audio->freq = 0;
    audio->context = NULL;
    kqt_Mix_state_init(&audio->state);
    audio->open = open;
    audio->close = close;
    audio->set_buffer_size = NULL;
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


char* Audio_get_error(Audio* audio)
{
    assert(audio != NULL);
    return audio->error;
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


bool Audio_set_buffer_size(Audio* audio, uint32_t nframes)
{
    assert(audio != NULL);
    if (audio->set_buffer_size == NULL)
    {
        Audio_set_error(audio, "Driver does not support buffer resizing");
        return false;
    }
    return audio->set_buffer_size(audio, nframes);
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


void Audio_pause(Audio* audio, bool pause)
{
    assert(audio != NULL);
    audio->pause = pause;
    return;
}


bool Audio_get_state(Audio* audio, kqt_Mix_state* state)
{
    assert(audio != NULL);
    assert(state != NULL);
    assert(audio->active);
    int err = pthread_mutex_lock(&audio->state_mutex);
    if (err != 0)
    {
        if (err == EINVAL)
        {
            fprintf(stderr, "\nAudio_get_state: invalid mutex\n");
        }
        else if (err == EDEADLK)
        {
            fprintf(stderr, "\nAudio_get_state: deadlock\n");
        }
        return false;
    }
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    if (now.tv_usec >= 500000)
    {
        timeout.tv_sec = now.tv_sec + 1;
        timeout.tv_nsec = (now.tv_usec - 500000) * 1000;
    }
    else
    {
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = (now.tv_usec + 500000) * 1000;
    }
    err = pthread_cond_timedwait(&audio->state_cond, &audio->state_mutex, &timeout);
    if (err != 0)
    {
        if (err == ETIMEDOUT)
        {
            fprintf(stderr, "\nAudio_get_state: timed out\n");
        }
        else if (err == EINTR)
        {
            fprintf(stderr, "\nAudio_get_state: interrupted\n");
        }
        err = pthread_mutex_unlock(&audio->state_mutex);
        if (err == EINVAL)
        {
            fprintf(stderr, "\nAudio_notify: invalid mutex\n");
        }
        else if (err == EPERM)
        {
            fprintf(stderr, "\nAudio_notify: unlocking mutex locked by someone else\n");
        }
        return false;
    }
    kqt_Mix_state_copy(state, &audio->state);
    err = pthread_mutex_unlock(&audio->state_mutex);
    if (err == EINVAL)
    {
        fprintf(stderr, "\nAudio_notify: invalid mutex\n");
        return false;
    }
    else if (err == EPERM)
    {
        fprintf(stderr, "\nAudio_notify: unlocking mutex locked by someone else\n");
        return false;
    }
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
        err = pthread_cond_broadcast(&audio->state_cond);
        if (err != 0)
        {
            fprintf(stderr, "\npthread_cond_broadcast failed: %s\n", strerror(err));
        }
        err = pthread_mutex_unlock(&audio->state_mutex);
        if (err == EINVAL)
        {
            fprintf(stderr, "\nAudio_notify: invalid mutex\n");
        }
        else if (err == EPERM)
        {
            fprintf(stderr, "\nAudio_notify: unlocking mutex locked by someone else\n");
        }
    }
    return err;
}


void Audio_set_error(Audio* audio, char* message, ...)
{
    assert(audio != NULL);
    assert(message != NULL);
    va_list args;
    va_start(args, message);
    vsnprintf(audio->error, AUDIO_ERROR_LENGTH, message, args);
    va_end(args);
    audio->error[AUDIO_ERROR_LENGTH - 1] = '\0';
    return;
}


