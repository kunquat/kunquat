

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
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <jack/jack.h>

#include <kunquat.h>

#include "demo_song.h"


typedef struct audio_callback_data
{
    bool active;
    Playlist* pl;
    uint32_t nframes;
    uint32_t freq;
    union
    {
        struct
        {
            jack_client_t* client;
            jack_port_t* ports[2];
        } jack;
    } driver;
} audio_callback_data;


/**
 * Creates a JACK client.
 *
 * \param user_data   Context data passed to callback functions -- must not be
 *                    \c NULL.
 *
 * \return   \c true if successful, or \c false if failed.
 */
static bool audio_init_jack(audio_callback_data* user_data);


/**
 * Closes the JACK client.
 *
 * \param user_data   Context data -- must not be \c NULL.
 */
void audio_close_jack(audio_callback_data* user_data);


int main(void)
{
    audio_callback_data* data =
            &(audio_callback_data){ .active = false,
                                    .pl = NULL,
                                    .nframes = 0,
                                    .freq = 0 };
    if (!audio_init_jack(data))
    {
        fprintf(stderr, "Couldn't initialise JACK\n");
        exit(EXIT_FAILURE);
    }
    Song* song = demo_song_create();
    if (song == NULL)
    {
        fprintf(stderr, "Couldn't create the demo Song\n");
        exit(EXIT_FAILURE);
    }
    int32_t song_id = Playlist_ins(data->pl, song);
    if (song_id == -1)
    {
        fprintf(stderr, "Couldn't add the demo Song into the Playlist\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Playing the demo, Enter to quit...\n");
    Playlist_play_song(data->pl, song_id);
    getchar();
    Playlist_stop_song(data->pl, song_id);
    audio_close_jack(data);
    exit(EXIT_SUCCESS);
}


static int audio_jack_bufsize(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    audio_callback_data* data = (audio_callback_data*)arg;
    if (!Playlist_set_buf_size(data->pl, nframes))
    {
        return -1;
    }
    return 0;
}


static int audio_jack_process(jack_nframes_t nframes, void* arg)
{
    assert(arg != NULL);
    audio_callback_data* data = (audio_callback_data*)arg;
    if (!data->active)
    {
        return 0;
    }
    jack_default_audio_sample_t* jbuf_l =
            jack_port_get_buffer(data->driver.jack.ports[0], nframes);
    jack_default_audio_sample_t* jbuf_r =
            jack_port_get_buffer(data->driver.jack.ports[1], nframes);
    jack_default_audio_sample_t* jbufs[2] = { jbuf_l, jbuf_r };
    for (uint32_t i = 0; i < nframes; ++i)
    {
        jbuf_l[i] = jbuf_r[i] = 0;
    }
    Playlist_mix(data->pl, nframes, jbufs);
    return 0;
}


static bool audio_init_jack(audio_callback_data* user_data)
{
    assert(user_data != NULL);
    user_data->pl = new_Playlist();
    if (user_data->pl == NULL)
    {
        return false;
    }
    user_data->driver.jack.client = NULL;
    user_data->driver.jack.ports[0] = user_data->driver.jack.ports[1] = NULL;
    jack_status_t status = 0;
    user_data->driver.jack.client = jack_client_open("Kunquat", JackNullOption, &status);
    if (user_data->driver.jack.client == NULL)
    {
        return false;
    }
    if (jack_set_process_callback(user_data->driver.jack.client,
            audio_jack_process, user_data) != 0)
    {
        user_data->driver.jack.client = NULL;
        return false;
    }
    if (jack_set_buffer_size_callback(user_data->driver.jack.client,
            audio_jack_bufsize, user_data) != 0)
    {
        user_data->driver.jack.client = NULL;
        return false;
    }
    user_data->driver.jack.ports[0] = jack_port_register(user_data->driver.jack.client,
            "out_l",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput | JackPortIsTerminal, 0);
    if (user_data->driver.jack.ports[0] == NULL)
    {
        audio_close_jack(user_data);
        return false;
    }
    user_data->driver.jack.ports[1] = jack_port_register(user_data->driver.jack.client,
            "out_r",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput | JackPortIsTerminal, 0);
    if (user_data->driver.jack.ports[1] == NULL)
    {
        audio_close_jack(user_data);
        return false;
    }
    user_data->nframes = jack_get_buffer_size(user_data->driver.jack.client);
    if (!Playlist_set_buf_size(user_data->pl, user_data->nframes))
    {
        audio_close_jack(user_data);
        return false;
    }
    if (jack_activate(user_data->driver.jack.client) != 0)
    {
        audio_close_jack(user_data);
        return false;
    }
    const char** available_ports = jack_get_ports(user_data->driver.jack.client,
            NULL,
            NULL,
            JackPortIsPhysical | JackPortIsInput);
    if (available_ports == NULL)
    {
        audio_close_jack(user_data);
        return false;
    }
    if (jack_connect(user_data->driver.jack.client,
            jack_port_name(user_data->driver.jack.ports[0]),
            available_ports[0]) != 0)
    {
        free(available_ports);
        audio_close_jack(user_data);
        return false;
    }
    if (jack_connect(user_data->driver.jack.client,
            jack_port_name(user_data->driver.jack.ports[1]),
            available_ports[1]) != 0)
    {
        free(available_ports);
        audio_close_jack(user_data);
        return false;
    }
    free(available_ports);
    user_data->freq = jack_get_sample_rate(user_data->driver.jack.client);
    Playlist_set_mix_freq(user_data->pl, user_data->freq);
    user_data->active = true;
    return true;
}


void audio_close_jack(audio_callback_data* user_data)
{
    assert(user_data != NULL);
    if (user_data->driver.jack.client != NULL)
    {
        jack_client_close(user_data->driver.jack.client);
        user_data->driver.jack.client = NULL;
    }
    user_data->driver.jack.ports[0] = user_data->driver.jack.ports[1] = NULL;
    if (user_data->pl != NULL)
    {
        del_Playlist(user_data->pl);
    }
    return;
}


