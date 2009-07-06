

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


#define _POSIX_C_SOURCE 199309L // for nanosleep

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include <pthread.h>

#include <time.h> // for nanosleep

#include <Audio.h>
#include <Audio_null.h>

#include <xmemory.h>


#define DEFAULT_BUF_SIZE (2048)


struct Audio_null
{
    Audio parent;
    bool thread_active;
    pthread_t play_thread;
};


static void* Audio_null_thread(void* data);

static int Audio_null_process(Audio_null* audio_null);

static bool Audio_null_open(Audio_null* audio_null);

static bool Audio_null_close(Audio_null* audio_null);

static void del_Audio_null(Audio_null* audio_null);


Audio* new_Audio_null(void)
{
    Audio_null* audio_null = xalloc(Audio_null);
    if (audio_null == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_null->parent,
                    (bool (*)(Audio*))Audio_null_open,
                    (bool (*)(Audio*))Audio_null_close,
                    (void (*)(Audio*))del_Audio_null))
    {
        xfree(audio_null);
        return NULL;
    }
    audio_null->thread_active = false;
    audio_null->parent.nframes = DEFAULT_BUF_SIZE;
    audio_null->parent.freq = 48000;
    return &audio_null->parent;
}


static bool Audio_null_open(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    if (audio_null->parent.active)
    {
        return false;
    }
    audio_null->parent.active = true;
    int err = pthread_create(&audio_null->play_thread, NULL, Audio_null_thread, audio_null);
    if (err != 0)
    {
        audio_null->parent.active = false;
        return false;
    }
    audio_null->thread_active = true;
    return true;
}


static bool Audio_null_close(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    if (!audio_null->parent.active)
    {
        return false;
    }
    audio_null->parent.active = false;
    int err = 0;
    if (audio_null->thread_active)
    {
        err = pthread_join(audio_null->play_thread, NULL);
        audio_null->thread_active = false;
    }
    if (err != 0)
    {
        return false;
    }
    return true;
}


static void* Audio_null_thread(void* data)
{
    assert(data != NULL);
    Audio_null* audio_null = data;
    while (audio_null->parent.active)
    {
        if (Audio_null_process(audio_null) < 0)
        {
            audio_null->parent.active = false;
        }
    }
    return NULL;
}


static int Audio_null_process(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    if (!audio_null->parent.active)
    {
        struct timespec delay;
        delay.tv_sec = 0;
        delay.tv_nsec = 50000000;
        nanosleep(&delay, NULL);
        Audio_notify(&audio_null->parent);
        return 0;
    }
    kqt_Context* context = audio_null->parent.context;
    if (context != NULL && !audio_null->parent.pause)
    {
        kqt_Context_mix(context,
                        audio_null->parent.nframes,
                        audio_null->parent.freq);
    }
    Audio_notify(&audio_null->parent);
    return 0;
}


void del_Audio_null(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    assert(!audio_null->parent.active);
    xfree(audio_null);
    return;
}


