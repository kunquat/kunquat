

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

#include <unistd.h>

#include <sys/time.h>
#include <pthread.h>

#include <Audio.h>
#include <kunquat/Handle.h>
#include <kunquat/Player_ext.h>

#include <Mix_state.h>

#include <xmemory.h>


bool Audio_init(Audio* audio,
                char* name,
                bool (*open)(Audio*),
                bool (*close)(Audio*),
                void (*destroy)(Audio*))
{
    assert(audio != NULL);
    assert(name != NULL);
    assert(open != NULL);
    assert(close != NULL);
    assert(destroy != NULL);
    audio->name = name;
    audio->active = false;
    memset(audio->error, '\0', AUDIO_ERROR_LENGTH);
    audio->pause = false;
    audio->nframes = 0;
    audio->freq = 0;
    audio->handle = NULL;
    Mix_state_init(&audio->state);
    audio->open = open;
    audio->close = close;
    audio->set_buffer_size = NULL;
    audio->set_freq = NULL;
    audio->set_frame_format = NULL;
    audio->set_file = NULL;
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


char* Audio_get_name(Audio* audio)
{
    assert(audio != NULL);
    return audio->name;
}


const char* Audio_get_full_name(Audio* audio)
{
    assert(audio != NULL);
    return audio->full_name;
}


char* Audio_get_error(Audio* audio)
{
    assert(audio != NULL);
    return audio->error;
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


bool Audio_set_freq(Audio* audio, uint32_t freq)
{
    assert(audio != NULL);
    assert(freq > 0);
    if (audio->set_freq == NULL)
    {
        Audio_set_error(audio, "Driver does not support changing the mixing frequency");
        return false;
    }
    return audio->set_freq(audio, freq);
}


bool Audio_set_frame_format(Audio* audio, char* format)
{
    assert(audio != NULL);
    assert(format != NULL);
    if (audio->set_frame_format == NULL)
    {
        Audio_set_error(audio, "Driver does not support changing the frame format");
        return false;
    }
    if (strlen(format) < 2 || strlen(format) > 3
            || !(format[0] == 'i' || format[1] == 'f')
            || !isdigit(format[1])
            || !(format[2] == '\0' || isdigit(format[2])))
    {
        Audio_set_error(audio, "Invalid frame format: %s", format);
    }
    return audio->set_frame_format(audio, format);
}


bool Audio_set_file(Audio* audio, char* path)
{
    assert(audio != NULL);
    assert(path != NULL);
    if (audio->set_file == NULL)
    {
        Audio_set_error(audio, "Driver does not support writing into an output file");
        return false;
    }
    return audio->set_file(audio, path);
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


void Audio_set_handle(Audio* audio, kqt_Handle* handle)
{
    assert(audio != NULL);
    audio->handle = handle;
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


bool Audio_get_state(Audio* audio, Mix_state* state)
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
    timeout.tv_sec = now.tv_sec + 1;
    timeout.tv_nsec = now.tv_usec * 1000;
    err = pthread_cond_timedwait(&audio->state_cond, &audio->state_mutex, &timeout);
    if (err != 0)
    {
        if (err == ETIMEDOUT)
        {
//            fprintf(stderr, "\nAudio_get_state: timed out\n");
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
    Mix_state_copy(state, &audio->state);
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
        if (audio->handle != NULL)
        {
            Mix_state_from_handle(&audio->state, audio->handle);
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


