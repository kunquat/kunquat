

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
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#include <pulse/simple.h>
#include <pulse/sample.h>
#include <pulse/error.h>

#include <pthread.h>

#include <Audio.h>
#include <Audio_pulse.h>

#include <kunquat/Player.h>

#include <xmemory.h>


#define DEFAULT_BUF_SIZE (2048)


struct Audio_pulse
{
    Audio parent;
    bool thread_active;
    pthread_t play_thread;
    pa_simple* server;
    pa_sample_spec spec;
    float* out_buf;
};


static void* Audio_pulse_thread(void* data);

static int Audio_pulse_process(Audio_pulse* audio_pulse);

static bool Audio_pulse_set_buffer_size(Audio_pulse* audio_pulse,
                                        uint32_t nframes);

static bool Audio_pulse_set_freq(Audio_pulse* audio_pulse, uint32_t freq);

static bool Audio_pulse_open(Audio_pulse* audio_pulse);

static bool Audio_pulse_close(Audio_pulse* audio_pulse);

static void del_Audio_pulse(Audio_pulse* audio_pulse);


Audio* new_Audio_pulse(void)
{
    Audio_pulse* audio_pulse = xalloc(Audio_pulse);
    if (audio_pulse == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_pulse->parent,
                    "pulse",
                    (bool (*)(Audio*))Audio_pulse_open,
                    (bool (*)(Audio*))Audio_pulse_close,
                    (void (*)(Audio*))del_Audio_pulse))
    {
        xfree(audio_pulse);
        return NULL;
    }
    audio_pulse->parent.set_buffer_size =
            (bool (*)(Audio*, uint32_t))Audio_pulse_set_buffer_size;
    audio_pulse->parent.set_freq =
            (bool (*)(Audio*, uint32_t))Audio_pulse_set_freq;
    audio_pulse->thread_active = false;
    audio_pulse->server = NULL;
    audio_pulse->out_buf = NULL;
    audio_pulse->spec.format = PA_SAMPLE_FLOAT32NE;
    audio_pulse->spec.rate = 48000;
    audio_pulse->spec.channels = 2;

    audio_pulse->parent.freq = audio_pulse->spec.rate;
    audio_pulse->parent.nframes = DEFAULT_BUF_SIZE;
    audio_pulse->out_buf = xnalloc(float, audio_pulse->parent.nframes * 2);
    if (audio_pulse->out_buf == NULL)
    {
        del_Audio(&audio_pulse->parent);
        return NULL;
    }
    return &audio_pulse->parent;
}


static bool Audio_pulse_open(Audio_pulse* audio_pulse)
{
    assert(audio_pulse != NULL);
    Audio* audio = &audio_pulse->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Driver is already active");
        return false;
    }

    int error = 0;
    audio_pulse->server = pa_simple_new(NULL, // default server
                                        "kunquat-player",
                                        PA_STREAM_PLAYBACK,
                                        NULL, // default device
                                        "Music",
                                        &audio_pulse->spec,
                                        NULL, // default channel map
                                        NULL, // default buffering attributes
                                        &error);
    if (audio_pulse->server == NULL)
    {
        Audio_set_error(audio, "Couldn't connect to PulseAudio: %s",
                pa_strerror(error));
        return false;
    }

    audio->active = true;
    int err = pthread_create(&audio_pulse->play_thread, NULL,
                             Audio_pulse_thread, audio_pulse);
    if (err != 0)
    {
        Audio_close(audio);
        Audio_set_error(audio, "Couldn't create audio thread"
                " for PulseAudio output");
        return false;
    }
    audio_pulse->thread_active = true;
    return true;
}


static bool Audio_pulse_set_buffer_size(Audio_pulse* audio_pulse,
                                        uint32_t nframes)
{
    assert(audio_pulse != NULL);
    assert(nframes > 0);
    Audio* audio = &audio_pulse->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set buffer size"
                " while the driver is active.");
        return false;
    }
    if (audio->handle != NULL)
    {
        if (!kqt_Handle_set_buffer_size(audio->handle, nframes))
        {
            Audio_set_error(audio, kqt_Handle_get_error(audio->handle));
            audio->handle = NULL;
            return false;
        }
    }
    float* new_buf = xrealloc(float, nframes * 2, audio_pulse->out_buf);
    if (new_buf == NULL)
    {
        Audio_set_error(audio, "Couldn't allocate memory for new output buffers.");
        return false;
    }
    audio_pulse->out_buf = new_buf;
    audio->nframes = nframes;
    return true;
}


static bool Audio_pulse_set_freq(Audio_pulse* audio_pulse, uint32_t freq)
{
    assert(audio_pulse != NULL);
    assert(freq > 0);
    Audio* audio = &audio_pulse->parent;
    if (freq > PA_RATE_MAX)
    {
        Audio_set_error(audio, "Cannot set the mixing frequency above"
                " the maximum allowed by PulseAudio (%d Hz).", PA_RATE_MAX);
        return false;
    }
    if (audio->active)
    {
        Audio_set_error(audio, "Cannot set mixing frequency while the driver is active.");
        return false;
    }
    audio->freq = freq;
    audio_pulse->spec.rate = freq;
    return true;
}


static void* Audio_pulse_thread(void* data)
{
    assert(data != NULL);
    Audio_pulse* audio_pulse = data;
    while (audio_pulse->parent.active)
    {
        if (Audio_pulse_process(audio_pulse) < 0)
        {
            fprintf(stderr, "PulseAudio driver callback failed\n");
            audio_pulse->parent.active = false;
        }
    }
    return NULL;
}


static int Audio_pulse_process(Audio_pulse* audio_pulse)
{
    assert(audio_pulse != NULL);
    Audio* audio = &audio_pulse->parent;
    if (!audio->active)
    {
        Audio_notify(audio);
        return 0;
    }
    uint32_t mixed = 0;
    assert(audio_pulse->out_buf != NULL);
    kqt_Handle* handle = audio->handle;
    if (handle != NULL && !audio->pause)
    {
        mixed = kqt_Handle_mix(handle, audio->nframes, audio->freq);
        float* bufs[KQT_BUFFERS_MAX] = { 
            kqt_Handle_get_buffer(handle, 0),
            kqt_Handle_get_buffer(handle, 1)
        };
        for (uint32_t i = 0; i < mixed; ++i)
        {
            audio_pulse->out_buf[i * 2] = bufs[0][i];
            audio_pulse->out_buf[(i * 2) + 1] = bufs[1][i];
        }
    }
    for (uint32_t i = mixed * 2; i < audio->nframes * 2; ++i)
    {
        audio_pulse->out_buf[i] = 0;
    }
    int error = 0;
    if (pa_simple_write(audio_pulse->server,
                        audio_pulse->out_buf,
                        sizeof(float) * audio->nframes * 2,
                        &error) < 0)
    {
        Audio_notify(audio);
        return -1;
    }
    Audio_notify(audio);
    return 0;
}


static bool Audio_pulse_close(Audio_pulse* audio_pulse)
{
    assert(audio_pulse != NULL);
    Audio* audio = &audio_pulse->parent;
    audio->active = false;
    if (audio_pulse->thread_active)
    {
        pthread_join(audio_pulse->play_thread, NULL);
        audio_pulse->thread_active = false;
    }
    if (audio_pulse->server != NULL)
    {
        bool ok = true;
        int error = 0;
        if (pa_simple_drain(audio_pulse->server, &error) < 0)
        {
            ok = false;
            Audio_set_error(audio, "Couldn't flush the audio stream: %s",
                    pa_strerror(error));
        }
        pa_simple_free(audio_pulse->server);
        audio_pulse->server = NULL;
        if (!ok)
        {
            return false;
        }
    }
    return true;
}


void del_Audio_pulse(Audio_pulse* audio_pulse)
{
    assert(audio_pulse != NULL);
    assert(!audio_pulse->parent.active);
    if (audio_pulse->out_buf != NULL)
    {
        xfree(audio_pulse->out_buf);
        audio_pulse->out_buf = NULL;
    }
    xfree(audio_pulse);
    return;
}


