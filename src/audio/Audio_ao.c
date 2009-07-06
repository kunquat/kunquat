

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
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <ao/ao.h>
#include <pthread.h>
#include <errno.h>

#include <Audio.h>
#include <Audio_ao.h>

#include <xmemory.h>


#define DEFAULT_BUF_SIZE (2048)


struct Audio_ao
{
    Audio parent;
    bool thread_active;
    pthread_t play_thread;
    ao_device* device;
    ao_sample_format format;
    short* out_buf;
};


static void* Audio_ao_thread(void* data);

static int Audio_ao_process(Audio_ao* audio_ao);

static bool Audio_ao_set_buffer_size(Audio_ao* audio_ao, uint32_t nframes);

static bool Audio_ao_set_freq(Audio_ao* audio_ao, uint32_t freq);

static bool Audio_ao_open(Audio_ao* audio_ao);

static bool Audio_ao_close(Audio_ao* audio_ao);

static void del_Audio_ao(Audio_ao* audio_ao);


Audio* new_Audio_ao(void)
{
    Audio_ao* audio_ao = xalloc(Audio_ao);
    if (audio_ao == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_ao->parent,
                    (bool (*)(Audio*))Audio_ao_open,
                    (bool (*)(Audio*))Audio_ao_close,
                    (void (*)(Audio*))del_Audio_ao))
    {
        xfree(audio_ao);
        return NULL;
    }
    audio_ao->parent.set_buffer_size = (bool (*)(Audio*, uint32_t))Audio_ao_set_buffer_size;
    audio_ao->parent.set_freq = (bool (*)(Audio*, uint32_t))Audio_ao_set_freq;
    audio_ao->thread_active = false;
    audio_ao->device = NULL;
    audio_ao->out_buf = NULL;
    audio_ao->format.bits = 16;
    audio_ao->format.rate = 44100;
    audio_ao->format.channels = 2;
    audio_ao->format.byte_format = AO_FMT_NATIVE;
    audio_ao->parent.freq = audio_ao->format.rate;
    audio_ao->parent.nframes = 0;
    audio_ao->out_buf = xnalloc(short, DEFAULT_BUF_SIZE * 2);
    if (audio_ao->out_buf == NULL)
    {
        del_Audio(&audio_ao->parent);
        return NULL;
    }
    audio_ao->parent.nframes = DEFAULT_BUF_SIZE;
    return &audio_ao->parent;
}


static bool Audio_ao_open(Audio_ao* audio_ao)
{
    assert(audio_ao != NULL);
    Audio* audio = &audio_ao->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Driver is already active");
        return false;
    }
    ao_initialize();
    int driver_id = ao_default_driver_id();
    if (driver_id == -1)
    {
        Audio_set_error(audio, "Couldn't find a usable audio device");
        ao_shutdown();
        return false;
    }
    errno = 0;
    audio_ao->device = ao_open_live(driver_id, &audio_ao->format, NULL);
    if (audio_ao->device == NULL)
    {
        Audio_close(&audio_ao->parent);
        switch (errno)
        {
            case AO_ENODRIVER:
                Audio_set_error(audio, "libao: Invalid driver ID");
                break;
            case AO_ENOTLIVE:
                Audio_set_error(audio, "libao: The device is not for live output");
                break;
            case AO_EBADOPTION:
                Audio_set_error(audio, "libao: Invalid option");
                break;
            case AO_EOPENDEVICE:
                Audio_set_error(audio, "libao: The device cannot be opened");
                break;
            case AO_EFAIL:
            default:
                Audio_set_error(audio, "libao initialisation failed");
                break;
        }
        return false;
    }
    audio_ao->parent.active = true;
    int err = pthread_create(&audio_ao->play_thread, NULL, Audio_ao_thread, audio_ao);
    if (err != 0)
    {
        Audio_close(&audio_ao->parent);
        Audio_set_error(audio, "Couldn't create audio thread for libao");
        return false;
    }
    audio_ao->thread_active = true;
    return true;
}


static bool Audio_ao_set_buffer_size(Audio_ao* audio_ao, uint32_t nframes)
{
    assert(audio_ao != NULL);
    assert(nframes > 0);
    Audio* audio = &audio_ao->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set buffer size while the driver is active.");
        return false;
    }
    if (audio->context != NULL)
    {
        if (kqt_Context_set_buffer_size(audio->context, nframes, NULL))
        {
            Audio_set_error(audio, "Couldn't allocate memory for Kunquat Context buffers.");
            return false;
        }
    }
    short* new_buf = xrealloc(short, nframes * 2, audio_ao->out_buf);
    if (new_buf == NULL)
    {
        Audio_set_error(audio, "Couldn't allocate memory for new buffers.");
        return false;
    }
    audio_ao->out_buf = new_buf;
    audio->nframes = nframes;
    return true;
}


static bool Audio_ao_set_freq(Audio_ao* audio_ao, uint32_t freq)
{
    assert(audio_ao != NULL);
    assert(freq > 0);
    Audio* audio = &audio_ao->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set mixing frequency while the driver is active.");
        return false;
    }
    audio->freq = freq;
    audio_ao->format.rate = freq;
    return true;
}


static void* Audio_ao_thread(void* data)
{
    assert(data != NULL);
    Audio_ao* audio_ao = data;
    while (audio_ao->parent.active)
    {
        if (Audio_ao_process(audio_ao) < 0)
        {
            fprintf(stderr, "libao callback failed\n");
            audio_ao->parent.active = false;
        }
    }
    return NULL;
}


static int Audio_ao_process(Audio_ao* audio_ao)
{
    assert(audio_ao != NULL);
    Audio* audio = &audio_ao->parent;
    if (!audio->active)
    {
        Audio_notify(audio);
        return 0;
    }
    uint32_t mixed = 0;
    assert(audio_ao->out_buf != NULL);
    kqt_Context* context = audio->context;
    if (context != NULL && !audio->pause)
    {
        mixed = kqt_Context_mix(context, audio->nframes, audio->freq);
        int buf_count = kqt_Context_get_buffer_count(context);
        kqt_frame** bufs = kqt_Context_get_buffers(context);
        for (uint32_t i = 0; i < mixed; ++i)
        {
            audio_ao->out_buf[i * 2] = (short)(bufs[0][i] * INT16_MAX);
            if (buf_count > 1)
            {
                audio_ao->out_buf[(i * 2) + 1] = (short)(bufs[1][i] * INT16_MAX);
            }
            else
            {
                audio_ao->out_buf[(i * 2) + 1] = audio_ao->out_buf[i * 2];
            }
        }
    }
    for (uint32_t i = mixed * 2; i < audio->nframes * 2; ++i)
    {
        audio_ao->out_buf[i] = 0;
    }
    if (!ao_play(audio_ao->device, (void*)audio_ao->out_buf, audio->nframes * 2 * 2))
    {
        Audio_notify(audio);
        return -1;
    }
    Audio_notify(audio);
    return 0;
}


static bool Audio_ao_close(Audio_ao* audio_ao)
{
    assert(audio_ao != NULL);
    Audio* audio = &audio_ao->parent;
    audio->active = false;
    if (audio_ao->device != NULL)
    {
        if (audio_ao->thread_active)
        {
            pthread_join(audio_ao->play_thread, NULL);
            audio_ao->thread_active = false;
        }
        int ok = ao_close(audio_ao->device);
        audio_ao->device = NULL;
        ao_shutdown();
        if (!ok)
        {
            Audio_set_error(audio, "An error occurred while closing the libao driver");
            return false;
        }
    }
    return true;
}


void del_Audio_ao(Audio_ao* audio_ao)
{
    assert(audio_ao != NULL);
    assert(!audio_ao->parent.active);
    if (audio_ao->out_buf != NULL)
    {
        xfree(audio_ao->out_buf);
        audio_ao->out_buf = NULL;
    }
    xfree(audio_ao);
    return;
}


