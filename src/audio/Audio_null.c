

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


#define _XOPEN_SOURCE 500 // for usleep

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include <pthread.h>

#include <unistd.h> // for usleep

#include <Audio.h>
#include <Audio_null.h>

#include <kunquat/Player.h>

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

static bool Audio_null_set_freq(Audio_null* audio_null, uint32_t freq);

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
                    "null",
                    (bool (*)(Audio*))Audio_null_open,
                    (bool (*)(Audio*))Audio_null_close,
                    (void (*)(Audio*))del_Audio_null))
    {
        xfree(audio_null);
        return NULL;
    }
    audio_null->parent.set_freq = (bool (*)(Audio*, uint32_t))Audio_null_set_freq;
    audio_null->thread_active = false;
    audio_null->parent.nframes = DEFAULT_BUF_SIZE;
    audio_null->parent.freq = 48000;
    return &audio_null->parent;
}


static bool Audio_null_set_freq(Audio_null* audio_null, uint32_t freq)
{
    assert(audio_null != NULL);
    assert(freq > 0);
    Audio* audio = &audio_null->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set mixing frequency while the driver is active.");
        return false;
    }
    audio->freq = freq;
    return true;
}


static bool Audio_null_open(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    Audio* audio = &audio_null->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Driver is already active");
        return false;
    }
    audio->active = true;
    int err = pthread_create(&audio_null->play_thread, NULL, Audio_null_thread, audio_null);
    if (err != 0)
    {
        audio->active = false;
        Audio_set_error(audio, "Couldn't create audio thread");
        return false;
    }
    audio_null->thread_active = true;
    return true;
}


static bool Audio_null_close(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    audio_null->parent.active = false;
    int err = 0;
    if (audio_null->thread_active)
    {
        err = pthread_join(audio_null->play_thread, NULL);
        audio_null->thread_active = false;
    }
    if (err != 0)
    {
        Audio_set_error(&audio_null->parent, "Couldn't close the null driver");
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
    Audio* audio = &audio_null->parent;
    if (!audio->active)
    {
        usleep(100000);
        Audio_notify(audio);
        return 0;
    }
    kqt_Handle* handle = audio->handle;
    if (handle != NULL && !audio->pause)
    {
        kqt_Handle_mix(handle, audio->nframes, audio->freq);
    }
    else
    {
        usleep(100000);
    }
    Audio_notify(audio);
    return 0;
}


void del_Audio_null(Audio_null* audio_null)
{
    assert(audio_null != NULL);
    assert(!audio_null->parent.active);
    xfree(audio_null);
    return;
}


