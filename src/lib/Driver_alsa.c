

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
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <alsa/asoundlib.h>
#include <pthread.h>

#include <Player.h>
#include <Playlist.h>
#include <Song.h>
#include "Driver_alsa.h"

#include <frame_t.h>


/**
 * Mixing callback function for ALSA.
 *
 * \param nframes   Number of frames to be mixed.
 *
 * \return   Zero on success, negative on error.
 */
static int Driver_alsa_process(snd_pcm_sframes_t nframes, Playlist* playlist);


/**
 * Playback thread function.
 *
 * \param data   Data passed.
 */
static void* Driver_alsa_thread(void* data);


/**
 * Playback thread.
 */
pthread_t play_thread;


/**
 * The ALSA client handle.
 */
static snd_pcm_t* handle = NULL;


/**
 * The buffer for outgoing data.
 */
static short out_buf[4096] = { 0 };


/**
 * Active status of the driver.
 */
static bool active = false;


bool Driver_alsa_init(Playlist* playlist, uint32_t* freq)
{
    snd_pcm_hw_params_t* hw_params = NULL;
    int err = 0;
    err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0)
    {
        handle = NULL;
        return false;
    }
    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0)
    {
        hw_params = NULL;
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_malloc()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_any(handle, hw_params);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_any()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_set_access(handle, hw_params,
            SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_access()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_format()\n");
        goto cleanup;
    }
    unsigned int fr = 44100;
    err = snd_pcm_hw_params_set_rate_near(handle, hw_params, &fr, NULL);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_rate_near()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_set_channels(handle, hw_params, 2);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_channels()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_set_periods(handle, hw_params, 2, 0);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_periods()\n");
        goto cleanup;
    }
    err = snd_pcm_hw_params_set_buffer_size(handle, hw_params, 4096);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params_set_buffer_size(): %d\n", err);
        goto cleanup;
    }
    err = snd_pcm_hw_params(handle, hw_params);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_hw_params()\n");
        goto cleanup;
    }
    snd_pcm_hw_params_free(hw_params);
    hw_params = NULL;
    err = snd_pcm_prepare(handle);
    if (err < 0)
    {
        fprintf(stderr, "ALSA driver failed at snd_pcm_prepare()\n");
        goto cleanup;
    }
    active = true;
    err = pthread_create(&play_thread, NULL, Driver_alsa_thread, playlist);
    if (err != 0)
    {
        active = false;
        fprintf(stderr, "ALSA driver failed at thread creation\n");
        goto cleanup;
    }
    *freq = 44100;
    return true;

cleanup:

    if (hw_params != NULL)
    {
        snd_pcm_hw_params_free(hw_params);
        hw_params = NULL;
    }
    if (handle != NULL)
    {
        snd_pcm_close(handle);
        handle = NULL;
    }
    return false;
}


void Driver_alsa_close(void)
{
    if (handle != NULL)
    {
        active = false;
        pthread_join(play_thread, NULL);
        snd_pcm_close(handle);
        handle = NULL;
    }
    return;
}


static void* Driver_alsa_thread(void* data)
{
    assert(handle != NULL);
    assert(data != NULL);
    Playlist* playlist = data;
    while (active)
    {
        snd_pcm_sframes_t to_be_mixed = 4096;
        if (to_be_mixed > 4096)
        {
            to_be_mixed = 4096;
        }
        if (Driver_alsa_process(to_be_mixed, playlist) < to_be_mixed)
        {
            fprintf(stderr, "callback failed\n");
            snd_pcm_prepare(handle);
        }
    }
    snd_pcm_drop(handle);
    return NULL;
}


static int Driver_alsa_process(snd_pcm_sframes_t nframes, Playlist* playlist)
{
    (void)playlist;
    int err = 0;
    static frame_t mix_l[8192];
    static frame_t mix_r[8192];
    for (int i = 0; i < 8192; ++i)
    {
        mix_l[i] = mix_r[i] = 0;
    }
    Player* player = playlist->first;
    while (player != NULL)
    {
        if (!player->play->mode)
        {
            player = player->next;
            continue;
        }
        assert(player->play->mode > STOP);
        assert(player->play->mode < PLAY_LAST);
        int buf_count = Song_get_buf_count(player->song);
        frame_t** bufs = Song_get_bufs(player->song);
        bufs[0] = mix_l;
        bufs[1] = mix_r;
        uint32_t mixed = Player_mix(player, nframes);
        if (buf_count == 1)
        {
            for (uint32_t i = 0; i < mixed; ++i)
            {
                mix_r[i] = mix_l[i];
            }
        }
        player = player->next;
    }
    for (int i = 0; i < 8192; ++i)
    {
        out_buf[i * 2] = (short)(mix_l[i] * 32767);
        out_buf[(i * 2) + 1] = (short)(mix_r[i] * 32767);
    }
    err = snd_pcm_writei(handle, out_buf, nframes);
    if (err < 0)
    {
        fprintf(stderr, "write failed: %s\n", snd_strerror(err));
    }
    return err;
}


