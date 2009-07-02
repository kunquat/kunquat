

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

#include <jack/jack.h>

#include <Audio.h>
#include <Audio_jack.h>

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
    if (audio_jack->parent.context == NULL)
    {
        return 0;
    }
    if (!kqt_Context_set_buffer_size(audio_jack->parent.context, nframes, NULL))
    {
        audio_jack->parent.context = NULL;
        return -1;
    }
    return 0;
}


static int Audio_jack_process(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    Audio_jack* audio_jack = (Audio_jack*)arg;
    if (!audio_jack->parent.active)
    {
        Audio_notify(&audio_jack->parent);
        return 0;
    }
    uint32_t mixed = 0;
    kqt_Context* context = audio_jack->parent.context;
    jack_default_audio_sample_t* jbuf_l =
            jack_port_get_buffer(audio_jack->ports[0], nframes);
    jack_default_audio_sample_t* jbuf_r =
            jack_port_get_buffer(audio_jack->ports[1], nframes);
    jack_default_audio_sample_t* jbufs[2] = { jbuf_l, jbuf_r };
    if (context != NULL)
    {
        mixed = kqt_Context_mix(context, nframes, audio_jack->parent.freq);
        int buf_count = kqt_Context_get_buffer_count(context);
        kqt_frame** bufs = kqt_Context_get_buffers(context);
        for (int i = 0; i < buf_count; ++i)
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
    Audio_notify(&audio_jack->parent);
    return 0;
}


static void del_Audio_jack(Audio_jack* audio_jack)
{
    assert(audio_jack != NULL);
    audio_jack->parent.active = false;
    if (audio_jack->client != NULL)
    {
        jack_client_close(audio_jack->client);
        audio_jack->client = NULL;
    }
    xfree(audio_jack);
    return;
}


#define destroy_if_fail(context, expr) do\
    {\
        if (!(expr))\
        {\
            del_Audio(&(context)->parent);\
            return NULL;\
        }\
    }\
    while (false)

Audio* new_Audio_jack(void)
{
    Audio_jack* audio_jack = xalloc(Audio_jack);
    if (audio_jack == NULL)
    {
        return NULL;
    }
    destroy_if_fail(audio_jack, Audio_init(&audio_jack->parent, 
                (void (*)(Audio*))del_Audio_jack));
    audio_jack->client = NULL;
    audio_jack->ports[0] = audio_jack->ports[1] = NULL;
    
    jack_status_t status = 0;
    audio_jack->client = jack_client_open("Kunquat", JackNullOption, &status);
    destroy_if_fail(audio_jack, audio_jack->client != NULL);

    destroy_if_fail(audio_jack, jack_set_process_callback(audio_jack->client,
            Audio_jack_process, audio_jack) == 0);
    destroy_if_fail(audio_jack, jack_set_buffer_size_callback(audio_jack->client,
            Audio_jack_bufsize, audio_jack) == 0);

    const char* port_names[] = { "out_l", "out_r" };
    for (int i = 0; i < 2; ++i)
    {
        audio_jack->ports[i] = jack_port_register(audio_jack->client,
                port_names[i],
                JACK_DEFAULT_AUDIO_TYPE,
                JackPortIsOutput | JackPortIsTerminal, 0);
        destroy_if_fail(audio_jack, audio_jack->ports[i] != NULL);
    }

    audio_jack->parent.nframes = jack_get_buffer_size(audio_jack->client);

    destroy_if_fail(audio_jack, jack_activate(audio_jack->client) == 0);

    const char** available_ports = jack_get_ports(audio_jack->client,
            NULL,
            NULL,
            JackPortIsPhysical | JackPortIsInput);
    destroy_if_fail(audio_jack, available_ports != NULL);

    for (int i = 0; i < 2; ++i)
    {
        if (jack_connect(audio_jack->client,
                jack_port_name(audio_jack->ports[i]),
                available_ports[i]) != 0)
        {
            free(available_ports);
            del_Audio_jack(audio_jack);
            return NULL;
        }
    }
    free(available_ports);
    audio_jack->parent.freq = jack_get_sample_rate(audio_jack->client);
    audio_jack->parent.active = true;
    return (Audio*)audio_jack;
}

#undef destroy_if_fail


