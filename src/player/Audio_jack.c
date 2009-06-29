

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

#include "Audio_jack.h"


typedef struct Audio_jack
{
    bool active;
    uint32_t nframes;
    uint32_t freq;
    jack_client_t* client;
    jack_port_t* ports[2];
    Playlist* pl;
} Audio_jack;


static Audio_jack* context = NULL;


static int audio_jack_bufsize(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    Audio_jack* context = (Audio_jack*)arg;
    if (!Playlist_set_buf_size(context->pl, nframes))
    {
        return -1;
    }
    return 0;
}


static int audio_jack_process(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    Audio_jack* context = (Audio_jack*)arg;
    if (!context->active)
    {
        return 0;
    }
    jack_default_audio_sample_t* jbuf_l =
            jack_port_get_buffer(context->ports[0], nframes);
    jack_default_audio_sample_t* jbuf_r =
            jack_port_get_buffer(context->ports[1], nframes);
    jack_default_audio_sample_t* jbufs[2] = { jbuf_l, jbuf_r };
    for (uint32_t i = 0; i < nframes; ++i)
    {
        jbuf_l[i] = jbuf_r[i] = 0;
    }
    Playlist_mix(context->pl, nframes, jbufs);
    return 0;
}


#define destroy_if_fail(context, expr) do\
    {\
        if (!(expr))\
        {\
            Audio_jack_close();\
            return false;\
        }\
    }\
    while (false)


bool Audio_jack_open(Playlist* pl)
{
    assert(pl != NULL);
    context = malloc(sizeof(Audio_jack));
    if (context == NULL)
    {
        return false;
    }
    context->active = false;
    context->nframes = 0;
    context->freq = 0;
    context->client = NULL;
    context->ports[0] = context->ports[1] = NULL;
    context->pl = pl;
    
    jack_status_t status = 0;
    context->client = jack_client_open("Kunquat", JackNullOption, &status);
    destroy_if_fail(context, context->client != NULL);

    destroy_if_fail(context, jack_set_process_callback(context->client,
            audio_jack_process, context) == 0);
    destroy_if_fail(context, jack_set_buffer_size_callback(context->client,
            audio_jack_bufsize, context) == 0);

    context->ports[0] = jack_port_register(context->client,
            "out_l",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput | JackPortIsTerminal, 0);
    destroy_if_fail(context, context->ports[0] != NULL);

    context->ports[1] = jack_port_register(context->client,
            "out_r",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput | JackPortIsTerminal, 0);
    destroy_if_fail(context, context->ports[1] != NULL);

    context->nframes = jack_get_buffer_size(context->client);
    destroy_if_fail(context, Playlist_set_buf_size(context->pl, context->nframes));

    destroy_if_fail(context, jack_activate(context->client) == 0);

    const char** available_ports = jack_get_ports(context->client,
            NULL,
            NULL,
            JackPortIsPhysical | JackPortIsInput);
    destroy_if_fail(context, available_ports != NULL);

    if (jack_connect(context->client,
            jack_port_name(context->ports[0]),
            available_ports[0]) != 0)
    {
        free(available_ports);
        Audio_jack_close();
        return false;
    }
    if (jack_connect(context->client,
            jack_port_name(context->ports[1]),
            available_ports[1]) != 0)
    {
        free(available_ports);
        Audio_jack_close();
        return false;
    }
    free(available_ports);
    context->freq = jack_get_sample_rate(context->client);
    Playlist_set_mix_freq(context->pl, context->freq);
    context->active = true;
    return true;
}


void Audio_jack_close()
{
    assert(context != NULL);
    context->pl = NULL;
    if (context->client != NULL)
    {
        jack_client_close(context->client);
        context->client = NULL;
    }
    context->ports[0] = context->ports[1] = NULL;
    free(context);
    context = NULL;
    return;
}


