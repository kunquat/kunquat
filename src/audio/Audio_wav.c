

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
#include <errno.h>
#include <string.h>

#include <pthread.h>

#include <unistd.h> // for usleep

#include <Audio.h>
#include <Audio_wav.h>

#include <xmemory.h>


#define DEFAULT_BUF_SIZE (2048)


struct Audio_wav
{
    Audio parent;
    bool thread_active;
    pthread_t play_thread;
    char* path;
    FILE* out;
    bool header_written;
    uint32_t chunk_size;
};


static void* Audio_wav_thread(void* data);

static int Audio_wav_process(Audio_wav* audio_wav);

static bool Audio_wav_set_file(Audio_wav* audio_wav, char* path);

static bool Audio_wav_set_freq(Audio_wav* audio_wav, uint32_t freq);

static bool Audio_wav_open(Audio_wav* audio_wav);

static bool Audio_wav_close(Audio_wav* audio_wav);

static void del_Audio_wav(Audio_wav* audio_wav);

static int write_le(FILE* out, int64_t num, int bytes);


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
    audio_wav->thread_active = false;
    audio_wav->parent.nframes = DEFAULT_BUF_SIZE;
    audio_wav->parent.freq = 48000;
    audio_wav->path = NULL;
    audio_wav->out = NULL;
    audio_wav->header_written = false;
    audio_wav->chunk_size = 0;
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
    audio_wav->out = fopen(audio_wav->path, "rb");
    if (audio_wav->out != NULL)
    {
        fclose(audio_wav->out);
        audio_wav->out = NULL;
        Audio_set_error(audio, "File %s already exists", audio_wav->path);
        return false;
    }
    errno = 0;
    audio_wav->out = fopen(audio_wav->path, "wb");
    if (audio_wav->out == NULL)
    {
        Audio_set_error(audio, "Couldn't create file %s: %s", audio_wav->path, strerror(errno));
        return false;
    }
    audio->active = true;
    int err = pthread_create(&audio_wav->play_thread, NULL, Audio_wav_thread, audio_wav);
    if (err != 0)
    {
        audio->active = false;
        Audio_set_error(audio, "Couldn't create audio thread");
        return false;
    }
    audio_wav->thread_active = true;
    return true;
}


static int write_le(FILE* out, int64_t num, int bytes)
{
    assert(out != NULL);
    assert(bytes > 0);
    while (bytes > 0)
    {
        if (fputc(num & 0xff, out) == EOF)
        {
            return EOF;
        }
        num >>= 8;
        --bytes;
    }
    return 0;
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
        if (audio_wav->header_written)
        {
            err = 0;
            errno = 0;
            if (fseek(audio_wav->out, 4, SEEK_SET) == -1)
            {
                err = 1;
                Audio_set_error(&audio_wav->parent,
                                "Couldn't write the chunk length: %s", strerror(errno));
            }
            errno = 0;
            if (write_le(audio_wav->out, audio_wav->chunk_size + 36, 4) == EOF)
            {
                err = 1;
                Audio_set_error(&audio_wav->parent,
                                "Couldn't write the chunk length");
            }
            errno = 0;
            if (fseek(audio_wav->out, 40, SEEK_SET) == -1)
            {
                err = 1;
                Audio_set_error(&audio_wav->parent,
                                "Couldn't write the chunk length: %s", strerror(errno));
            }
            errno = 0;
            if (write_le(audio_wav->out, audio_wav->chunk_size, 4) == EOF)
            {
                err = 1;
                Audio_set_error(&audio_wav->parent,
                                "Couldn't write the chunk length");
            }
            if (err != 0)
            {
                fclose(audio_wav->out);
                audio_wav->out = NULL;
                return false;
            }
        }
        errno = 0;
        if (fclose(audio_wav->out) == EOF)
        {
            Audio_set_error(&audio_wav->parent, "Couldn't close the file %s: %s",
                            audio_wav->path, strerror(errno));
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


#define close_if_eof(audio_wav, operation)                            \
    do                                                                \
    {                                                                 \
        if ((operation) == EOF)                                       \
        {                                                             \
            Audio_set_error(&(audio_wav)->parent,                     \
                            "Couldn't write into the output file %s", \
                            (audio_wav)->path);                       \
            fclose((audio_wav)->out);                                 \
            (audio_wav)->out = NULL;                                  \
            (audio_wav)->parent.active = false;                       \
        }                                                             \
    } while (false)

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
    kqt_Context* context = audio->context;
    if (context != NULL && !audio->pause)
    {
        int buf_count = kqt_Context_get_buffer_count(context);
        int bits = 16;
        if (!audio_wav->header_written)
        {
            close_if_eof(audio_wav, fputs("RIFF", audio_wav->out));
            // Write zero chunk size for now
            // -- come back when we finish writing
            close_if_eof(audio_wav, write_le(audio_wav->out, 0, 4));
            close_if_eof(audio_wav, fputs("WAVE", audio_wav->out));

            close_if_eof(audio_wav, fputs("fmt ", audio_wav->out));   // subchunk ID
            close_if_eof(audio_wav, write_le(audio_wav->out, 16, 4)); // fmt subchunk size
            close_if_eof(audio_wav, write_le(audio_wav->out, 1, 2));  // format: PCM
            close_if_eof(audio_wav, write_le(audio_wav->out, buf_count, 2));
            close_if_eof(audio_wav, write_le(audio_wav->out, audio->freq, 4));
            close_if_eof(audio_wav, write_le(audio_wav->out,
                         audio->freq * buf_count * bits / 8, 4));     // byte rate
            close_if_eof(audio_wav, write_le(audio_wav->out, buf_count * bits / 8, 2));
            close_if_eof(audio_wav, write_le(audio_wav->out, bits, 2));

            close_if_eof(audio_wav, fputs("data", audio_wav->out));
            // Write zero chunk size for now
            // -- come back when we finish writing
            close_if_eof(audio_wav, write_le(audio_wav->out, 0, 4));
            audio_wav->header_written = true;
            audio_wav->chunk_size = 0;
        }
        uint32_t mixed = kqt_Context_mix(context, audio->nframes, audio->freq);
        kqt_frame** bufs = kqt_Context_get_buffers(context);
        for (uint32_t i = 0; i < mixed; ++i)
        {
            for (int k = 0; k < buf_count; ++k)
            {
                close_if_eof(audio_wav, write_le(audio_wav->out, 
                             bufs[k][i] * INT16_MAX, 2));
            }
        }
        audio_wav->chunk_size += mixed * buf_count * bits / 8;
    }
    else
    {
        usleep(100000);
    }
    Audio_notify(audio);
    return 0;
}

#undef close_if_eof


static void del_Audio_wav(Audio_wav* audio_wav)
{
    assert(audio_wav != NULL);
    assert(!audio_wav->parent.active);
    xfree(audio_wav);
    return;
}


