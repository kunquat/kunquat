

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <jack/jack.h>

#include <Audio.h>
#include <Audio_jack.h>

#include <kunquat/Player.h>

#include <xmemory.h>


struct Audio_jack
{
    Audio parent;
    jack_client_t* client;
    jack_port_t* ports[2];
};


static int Audio_jack_bufsize(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    Audio_jack* audio_jack = (Audio_jack*)arg;
    Audio* audio = &audio_jack->parent;
    if (audio->handle == NULL)
    {
        return 0;
    }
    if (!kqt_Handle_set_buffer_size(audio->handle, nframes))
    {
        Audio_set_error(audio, kqt_Handle_get_error(audio->handle));
        audio->handle = NULL;
        return -1;
    }
    audio->nframes = nframes;
    return 0;
}


static int Audio_jack_process(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    Audio_jack* audio_jack = (Audio_jack*)arg;
    Audio* audio = &audio_jack->parent;
    if (!audio->active)
    {
        Audio_notify(audio);
        return 0;
    }
    uint32_t mixed = 0;
    kqt_Handle* handle = audio->handle;
    jack_default_audio_sample_t* jbuf_l =
            jack_port_get_buffer(audio_jack->ports[0], nframes);
    jack_default_audio_sample_t* jbuf_r =
            jack_port_get_buffer(audio_jack->ports[1], nframes);
    jack_default_audio_sample_t* jbufs[2] = { jbuf_l, jbuf_r };
    if (handle != NULL && !audio->pause)
    {
        mixed = kqt_Handle_mix(handle, nframes, audio->freq);
        kqt_frame* bufs[KQT_BUFFERS_MAX] = {
            kqt_Handle_get_buffer(handle, 0),
            kqt_Handle_get_buffer(handle, 1)
        };
        for (int i = 0; i < 2; ++i)
        {
            for (uint32_t k = 0; k < mixed; ++k)
            {
                jbufs[i][k] = bufs[i][k];
            }
        }
    }
    for (uint32_t i = mixed; i < nframes; ++i)
    {
        jbuf_l[i] = jbuf_r[i] = 0;
    }
    Audio_notify(audio);
    return 0;
}


static bool Audio_jack_open(Audio_jack* audio_jack);

static bool Audio_jack_close(Audio_jack* audio_jack);

static void del_Audio_jack(Audio_jack* audio_jack);


Audio* new_Audio_jack(void)
{
    Audio_jack* audio_jack = xalloc(Audio_jack);
    if (audio_jack == NULL)
    {
        return NULL;
    }
    if (!Audio_init(&audio_jack->parent,
                    "jack",
                    (bool (*)(Audio*))Audio_jack_open,
                    (bool (*)(Audio*))Audio_jack_close,
                    (void (*)(Audio*))del_Audio_jack))
    {
        xfree(audio_jack);
        return NULL;
    }
    audio_jack->client = NULL;
    audio_jack->ports[0] = audio_jack->ports[1] = NULL;
    return &audio_jack->parent;
}


static bool Audio_jack_open(Audio_jack* audio_jack)
{
    assert(audio_jack != NULL);
    Audio* audio = &audio_jack->parent;
    if (audio->active)
    {
        Audio_set_error(audio, "Driver is already active");
        return false;
    }
    
    jack_status_t status = 0;
    audio_jack->client = jack_client_open("Kunquat", JackNullOption, &status);
    if (audio_jack->client == NULL)
    {
        Audio_set_error(audio, "Couldn't register as a JACK client");
        return false;
    }

    if (jack_set_process_callback(audio_jack->client,
                                  Audio_jack_process,
                                  audio_jack) != 0)
    {
        Audio_jack_close(audio_jack);
        Audio_set_error(audio, "Couldn't set JACK process callback");
        return false;
    }
    if (jack_set_buffer_size_callback(audio_jack->client,
                                      Audio_jack_bufsize,
                                      audio_jack) != 0)
    {
        Audio_jack_close(audio_jack);
        Audio_set_error(audio, "Couldn't set JACK buffer size callback");
        return false;
    }

    const char* port_names[] = { "out_l", "out_r" };
    for (int i = 0; i < 2; ++i)
    {
        audio_jack->ports[i] = jack_port_register(audio_jack->client,
                port_names[i],
                JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsOutput | JackPortIsTerminal, 0);
        if (audio_jack->ports[i] == NULL)
        {
            Audio_jack_close(audio_jack);
            Audio_set_error(audio, "Couldn't register port %s", port_names[i]);
            return false;
        }
    }

    audio_jack->parent.nframes = jack_get_buffer_size(audio_jack->client);

    if (jack_activate(audio_jack->client) != 0)
    {
        Audio_jack_close(audio_jack);
        Audio_set_error(audio, "Couldn't activate JACK");
        return false;
    }

    const char** available_ports = jack_get_ports(audio_jack->client,
            NULL,
            NULL,
            JackPortIsPhysical | JackPortIsInput);
    if (available_ports == NULL)
    {
        Audio_jack_close(audio_jack);
        Audio_set_error(audio, "Couldn't retrieve JACK input ports");
        return false;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (jack_connect(audio_jack->client,
                jack_port_name(audio_jack->ports[i]),
                available_ports[i]) != 0)
        {
            free(available_ports);
            Audio_jack_close(audio_jack);
            Audio_set_error(audio, "Couldn't connect port %s", port_names[i]);
            return NULL;
        }
    }
    free(available_ports);
    audio_jack->parent.freq = jack_get_sample_rate(audio_jack->client);
    audio_jack->parent.active = true;
    return true;
}


static bool Audio_jack_close(Audio_jack* audio_jack)
{
    assert(audio_jack != NULL);
    audio_jack->parent.active = false;
    if (audio_jack->client != NULL)
    {
        jack_client_close(audio_jack->client);
        audio_jack->client = NULL;
    }
    audio_jack->ports[0] = audio_jack->ports[1] = NULL;
    return true;
}


static void del_Audio_jack(Audio_jack* audio_jack)
{
    assert(audio_jack != NULL);
    assert(!audio_jack->parent.active);
    xfree(audio_jack);
    return;
}


