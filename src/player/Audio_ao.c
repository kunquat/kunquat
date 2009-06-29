

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

#include <kunquat.h>

#include "Audio_ao.h"


#define DEFAULT_BUF_SIZE (2048)


typedef struct Audio_ao
{
    bool active;
    bool thread_active;
    uint32_t nframes;
    uint32_t freq;
    pthread_t play_thread;
    ao_device* device;
    ao_sample_format format;
    Playlist* pl;
    short* out_buf;
    frame_t* bufs[2];
} Audio_ao;


static Audio_ao context_;
static Audio_ao* context = &context_;


static void* Audio_ao_thread(void* data);


static int Audio_ao_process(uint32_t nframes, Audio_ao* context);


bool Audio_ao_open(Playlist* pl)
{
    assert(pl != NULL);
    context->pl = pl;
    context->active = false;
    context->thread_active = false;
    context->nframes = 0;
    context->freq = 0;
    context->device = NULL;
    context->out_buf = NULL;
    context->bufs[0] = context->bufs[1] = NULL;
    ao_initialize();
    int driver_id = ao_default_driver_id();
    if (driver_id == -1)
    {
        fprintf(stderr, "Couldn't find a usable audio device.\n");
        ao_shutdown();
        return false;
    }
    context->format.bits = 16;
    context->format.rate = 44100;
    context->format.channels = 2;
    context->format.byte_format = AO_FMT_NATIVE;
    errno = 0;
    context->nframes = 0;
    context->bufs[0] = context->bufs[1] = NULL;
    context->out_buf = malloc(sizeof(short) * DEFAULT_BUF_SIZE * 2);
    if (context->out_buf == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for the audio buffer.\n");
        Audio_ao_close();
        ao_shutdown();
        return false;
    }
    for (int i = 0; i < 2; ++i)
    {
        context->bufs[i] = malloc(sizeof(frame_t) * DEFAULT_BUF_SIZE);
        if (context->bufs[i] == NULL)
        {
            fprintf(stderr, "Couldn't allocate memory for the mixing buffer.\n");
            Audio_ao_close();
            ao_shutdown();
            return false;
        }
    }
    context->nframes = DEFAULT_BUF_SIZE;
    if (!Playlist_set_buf_size(context->pl, context->nframes))
    {
        fprintf(stderr, "Couldn't allocate memory for mixing buffers.\n");
        Audio_ao_close();
        ao_shutdown();
        return false;
    }
    context->device = ao_open_live(driver_id, &context->format, NULL);
    if (context->device == NULL)
    {
        switch (errno)
        {
            case AO_ENODRIVER:
                fprintf(stderr, "libao: Invalid driver ID.\n");
                break;
            case AO_ENOTLIVE:
                fprintf(stderr, "libao: The device is not for live output.\n");
                break;
            case AO_EBADOPTION:
                fprintf(stderr, "libao: Invalid option.\n");
                break;
            case AO_EOPENDEVICE:
                fprintf(stderr, "libao: The device cannot be opened.\n");
                break;
            case AO_EFAIL:
            default:
                fprintf(stderr, "libao initialisation failed.\n");
                break;
        }
        Audio_ao_close();
        ao_shutdown();
        return false;
    }
    context->active = true;
    int err = pthread_create(&context->play_thread, NULL, Audio_ao_thread, context);
    if (err != 0)
    {
        fprintf(stderr, "Couldn't create audio thread for libao.\n");
        Audio_ao_close();
        return false;
    }
    context->thread_active = true;
    context->freq = context->format.rate;
    Playlist_set_mix_freq(context->pl, context->freq);
    return true;
}


static void* Audio_ao_thread(void* data)
{
    assert(context->device != NULL);
    assert(data != NULL);
    Audio_ao* context = data;
    while (context->active)
    {
        if (Audio_ao_process(context->nframes, context) < 0)
        {
            fprintf(stderr, "libao callback failed\n");
            context->active = false;
        }
    }
    return NULL;
}


static int Audio_ao_process(uint32_t nframes, Audio_ao* context)
{
    assert(context != NULL);
    if (!context->active)
    {
        return 0;
    }
    assert(context->bufs[0] != NULL);
    assert(context->bufs[1] != NULL);
    assert(context->out_buf != NULL);
    for (uint32_t i = 0; i < context->nframes; ++i)
    {
        context->bufs[0][i] = context->bufs[1][i] = 0;
    }
    Playlist_mix(context->pl, nframes, context->bufs);
    for (uint32_t i = 0; i < context->nframes; ++i)
    {
        context->out_buf[i * 2] = (short)(context->bufs[0][i] * 32767);
        context->out_buf[(i * 2) + 1] = (short)(context->bufs[1][i] * 32767);
    }
    if (!ao_play(context->device, (void*)context->out_buf, context->nframes * 2 * 2))
    {
        return -1;
    }
    return 0;
}


void Audio_ao_close(void)
{
    if (context->device != NULL)
    {
        context->active = false;
        if (context->thread_active)
        {
            pthread_join(context->play_thread, NULL);
            context->thread_active = false;
        }
        int ok = ao_close(context->device);
        context->device = NULL;
        if (!ok)
        {
            fprintf(stderr, "An error occurred while closing the libao driver.\n");
        }
        ao_shutdown();
    }
    if (context->out_buf != NULL)
    {
        free(context->out_buf);
        context->out_buf = NULL;
    }
    for (int i = 0; i < 2; ++i)
    {
        if (context->bufs[i] != NULL)
        {
            free(context->bufs[i]);
            context->bufs[i] = NULL;
        }
    }
    context->pl = NULL;
    return;
}


