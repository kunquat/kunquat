

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


#define _XOPEN_SOURCE 500 // for usleep

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <pthread.h>

#include <sndfile.h>

#include <unistd.h> // for usleep

#include <Audio.h>
#include <Audio_wav.h>

#include <kunquat/Player.h>

#include <xmemory.h>


#define DEFAULT_BUF_SIZE (2048)


struct Audio_wav
{
    Audio parent;
    bool thread_active;
    pthread_t play_thread;
    char* path;
    SF_INFO sfinfo;
    SNDFILE* out;
    float* out_buf;
};


static void* Audio_wav_thread(void* data);

static int Audio_wav_process(Audio_wav* audio_wav);

static bool Audio_wav_set_file(Audio_wav* audio_wav, char* path);

static bool Audio_wav_set_freq(Audio_wav* audio_wav, uint32_t freq);

static bool Audio_wav_set_frame_format(Audio_wav* audio_wav, char* format);

static bool Audio_wav_open(Audio_wav* audio_wav);

static bool Audio_wav_close(Audio_wav* audio_wav);

static void del_Audio_wav(Audio_wav* audio_wav);


Audio* new_Audio_wav(void)
{
    Audio_wav* audio_wav = xalloc(Audio_wav);
    if (audio_wav == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_wav->parent,
                    "wav",
                    (bool (*)(Audio*))Audio_wav_open,
                    (bool (*)(Audio*))Audio_wav_close,
                    (void (*)(Audio*))del_Audio_wav))
    {
        xfree(audio_wav);
        return NULL;
    }
    audio_wav->parent.set_file = (bool (*)(Audio*, char*))Audio_wav_set_file;
    audio_wav->parent.set_freq = (bool (*)(Audio*, uint32_t))Audio_wav_set_freq;
    audio_wav->parent.set_frame_format = (bool (*)(Audio*, char*))Audio_wav_set_frame_format;
    audio_wav->thread_active = false;
    audio_wav->parent.nframes = DEFAULT_BUF_SIZE;
    audio_wav->parent.freq = 48000;
    audio_wav->path = NULL;
    audio_wav->sfinfo.frames = 0;
    audio_wav->sfinfo.samplerate = audio_wav->parent.freq;
    audio_wav->sfinfo.channels = 2;
    audio_wav->sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    audio_wav->out = NULL;
    audio_wav->out_buf = xnalloc(float, audio_wav->parent.nframes * audio_wav->sfinfo.channels);
    if (audio_wav->out_buf == NULL)
    {
        del_Audio(&audio_wav->parent);
        return NULL;
    }
    return &audio_wav->parent;
}


static bool Audio_wav_set_file(Audio_wav* audio_wav, char* path)
{
    assert(audio_wav != NULL);
    assert(path != NULL);
    Audio* audio = &audio_wav->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set the output while the driver is active.");
        return false;
    }
    audio_wav->path = path;
    return true;
}


static bool Audio_wav_set_freq(Audio_wav* audio_wav, uint32_t freq)
{
    assert(audio_wav != NULL);
    assert(freq > 0);
    Audio* audio = &audio_wav->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set mixing frequency while the driver is active.");
        return false;
    }
    audio->freq = freq;
    audio_wav->sfinfo.samplerate = audio->freq;
    return true;
}


static bool Audio_wav_set_frame_format(Audio_wav* audio_wav, char* format)
{
    assert(audio_wav != NULL);
    assert(format != NULL);
    assert(strlen(format) >= 2);
    assert(strlen(format) <= 3);
    assert(format[0] == 'i' || format[0] == 'f');
    assert(isdigit(format[1]));
    assert(format[2] == '\0' || isdigit(format[2]));
    if (strcmp(format, "i8") == 0)
    {
        audio_wav->sfinfo.format &= ~SF_FORMAT_SUBMASK;
        audio_wav->sfinfo.format |= SF_FORMAT_PCM_U8;
    }
    else if (strcmp(format, "i16") == 0)
    {
        audio_wav->sfinfo.format &= ~SF_FORMAT_SUBMASK;
        audio_wav->sfinfo.format |= SF_FORMAT_PCM_16;
    }
    else if (strcmp(format, "i24") == 0)
    {
        audio_wav->sfinfo.format &= ~SF_FORMAT_SUBMASK;
        audio_wav->sfinfo.format |= SF_FORMAT_PCM_24;
    }
    else if (strcmp(format, "i32") == 0)
    {
        audio_wav->sfinfo.format &= ~SF_FORMAT_SUBMASK;
        audio_wav->sfinfo.format |= SF_FORMAT_PCM_32;
    }
    else if (strcmp(format, "f32") == 0)
    {
        audio_wav->sfinfo.format &= ~SF_FORMAT_SUBMASK;
        audio_wav->sfinfo.format |= SF_FORMAT_FLOAT;
    }
    else
    {
        Audio_set_error(&audio_wav->parent, "Unsupported frame format: %s", format);
        return false;
    }
    return true;
}


static bool Audio_wav_open(Audio_wav* audio_wav)
{
    assert(audio_wav != NULL);
    Audio* audio = &audio_wav->parent;
    if (audio_wav->path == NULL)
    {
        Audio_set_error(audio, "Driver requires an output file name");
        return false;
    }
    if (!sf_format_check(&audio_wav->sfinfo))
    {
        Audio_set_error(audio, "Invalid format parameters");
        return false;
    }
    FILE* test = fopen(audio_wav->path, "rb");
    if (test != NULL)
    {
        fclose(test);
        Audio_set_error(audio, "File %s already exists", audio_wav->path);
        return false;
    }
    audio_wav->out = sf_open(audio_wav->path, SFM_WRITE, &audio_wav->sfinfo);
    if (audio_wav->out == NULL)
    {
        Audio_set_error(audio, "Couldn't create file %s: %s",
                        audio_wav->path, sf_strerror(NULL));
        return false;
    }
    audio->active = true;
    int err = pthread_create(&audio_wav->play_thread, NULL, Audio_wav_thread, audio_wav);
    if (err != 0)
    {
        audio->active = false;
        Audio_set_error(audio, "Couldn't create audio thread");
        sf_close(audio_wav->out);
        audio_wav->out = NULL;
        return false;
    }
    audio_wav->thread_active = true;
    return true;
}


static bool Audio_wav_close(Audio_wav* audio_wav)
{
    assert(audio_wav != NULL);
    audio_wav->parent.active = false;
    int err = 0;
    if (audio_wav->thread_active)
    {
        err = pthread_join(audio_wav->play_thread, NULL);
        audio_wav->thread_active = false;
    }
    if (err != 0)
    {
        Audio_set_error(&audio_wav->parent, "Couldn't close the WAV driver");
        return false;
    }
    if (audio_wav->out != NULL)
    {
        err = sf_close(audio_wav->out);
        if (err != 0)
        {
            Audio_set_error(&audio_wav->parent, "Couldn't close the output file: %s",
                            sf_error_number(err));
            audio_wav->out = NULL;
            return false;
        }
        audio_wav->out = NULL;
    }
    return true;
}


static void* Audio_wav_thread(void* data)
{
    assert(data != NULL);
    Audio_wav* audio_wav = data;
    while (audio_wav->parent.active)
    {
        if (Audio_wav_process(audio_wav) < 0)
        {
            audio_wav->parent.active = false;
        }
    }
    return NULL;
}


static int Audio_wav_process(Audio_wav* audio_wav)
{
    assert(audio_wav != NULL);
    Audio* audio = &audio_wav->parent;
    if (!audio->active)
    {
        usleep(100000);
        Audio_notify(audio);
        return 0;
    }
    assert(audio_wav->out != NULL);
    assert(audio_wav->out_buf != NULL);
    kqt_Handle* handle = audio->handle;
    if (handle != NULL && !audio->pause)
    {
        uint32_t mixed = kqt_Handle_mix(handle, audio->nframes, audio->freq);
        int buf_count = kqt_Handle_get_buffer_count(handle);
        kqt_frame* bufs[KQT_BUFFERS_MAX] = {
            kqt_Handle_get_buffer(handle, 0),
            kqt_Handle_get_buffer(handle, 1)
        };
        for (uint32_t i = 0; i < mixed; ++i)
        {
            audio_wav->out_buf[i * 2] = (float)bufs[0][i];
            if (buf_count > 1)
            {
                audio_wav->out_buf[(i * 2) + 1] = (float)bufs[1][i];
            }
            else
            {
                audio_wav->out_buf[(i * 2) + 1] = audio_wav->out_buf[i * 2];
            }
        }
        sf_writef_float(audio_wav->out, audio_wav->out_buf, mixed);
    }
    else
    {
        usleep(100000);
    }
    Audio_notify(audio);
    return 0;
}


static void del_Audio_wav(Audio_wav* audio_wav)
{
    assert(audio_wav != NULL);
    assert(!audio_wav->parent.active);
    if (audio_wav->out_buf != NULL)
    {
        xfree(audio_wav->out_buf);
        audio_wav->out_buf = NULL;
    }
    xfree(audio_wav);
    return;
}


