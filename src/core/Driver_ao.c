

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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

#include <ao/ao.h>
#include <pthread.h>
#include <errno.h>

#include <Player.h>
#include <Playlist.h>
#include <Song.h>
#include "Driver_ao.h"

#include <frame_t.h>


/**
 * Mixing callback function for libao.
 *
 * \param nframes   Number of frames to be mixed.
 *
 * \return   Zero on success, negative on error.
 */
static int Driver_ao_process(uint32_t nframes, Playlist* playlist);


/**
 * Playback thread function.
 *
 * \param data   Data passed.
 */
static void* Driver_ao_thread(void* data);


/**
 * Playback thread.
 */
static pthread_t play_thread;


/**
 * The ao device.
 */
static ao_device* device = NULL;


/**
 * Sample format description.
 */
static ao_sample_format format;


/**
 * The buffer size in samples.
 */
#define K_AO_BUF_SIZE (4096)


/**
 * The buffer for outgoing data.
 */
static short out_buf[K_AO_BUF_SIZE * 2] = { 0 };


/**
 * Active status of the driver.
 */
static bool active = false;


bool Driver_ao_init(Playlist* playlist, uint32_t* freq)
{
	assert(playlist != NULL);
	assert(freq != NULL);
	ao_initialize();
	int driver_id = ao_default_driver_id();
	if (driver_id == -1)
	{
		fprintf(stderr, "Couldn't find a usable audio device.\n");
		ao_shutdown();
		return false;
	}
	format.bits = 16;
	format.rate = 44100;
	format.channels = 2;
	format.byte_format = AO_FMT_NATIVE;
	errno = 0;
	device = ao_open_live(driver_id, &format, NULL);
	if (device == NULL)
	{
		switch (errno)
		{
			case AO_ENODRIVER:
				fprintf(stderr, "Invalid driver ID.\n");
				break;
			case AO_ENOTLIVE:
				fprintf(stderr, "The device is not for live output.\n");
				break;
			case AO_EBADOPTION:
				fprintf(stderr, "Invalid option.\n");
				break;
			case AO_EOPENDEVICE:
				fprintf(stderr, "The device cannot be opened.\n");
				break;
			case AO_EFAIL:
			default:
				fprintf(stderr, "libao initialisation failed.\n");
				break;
		}
		ao_shutdown();
		return false;
	}
	active = true;
	int err = pthread_create(&play_thread, NULL, Driver_ao_thread, playlist);
	if (err != 0)
	{
		active = false;
		fprintf(stderr, "Couldn't create audio thread for libao.\n");
		ao_close(device);
		ao_shutdown();
		return false;
	}
	*freq = format.rate;
	return true;
}


void Driver_ao_close(void)
{
	assert(device != NULL);
	if (device != NULL)
	{
		active = false;
		pthread_join(play_thread, NULL);
		int ok = ao_close(device);
		device = NULL;
		if (!ok)
		{
			fprintf(stderr, "An error occurred while closing the libao driver.\n");
		}
		ao_shutdown();
	}
	return;
}


static void* Driver_ao_thread(void* data)
{
	assert(device != NULL);
	assert(data != NULL);
	Playlist* playlist = data;
	while (active)
	{
		if (Driver_ao_process(K_AO_BUF_SIZE, playlist) < 0)
		{
			fprintf(stderr, "callback failed\n");
			active = false;
		}
	}
	return NULL;
}


static int Driver_ao_process(uint32_t nframes, Playlist* playlist)
{
	assert(playlist != NULL);
	static frame_t mix_l[K_AO_BUF_SIZE];
	static frame_t mix_r[K_AO_BUF_SIZE];
	for (uint32_t i = 0; i < K_AO_BUF_SIZE; ++i)
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
	for (uint32_t i = 0; i < K_AO_BUF_SIZE; ++i)
	{
		out_buf[i * 2] = (short)(mix_l[i] * 32767);
		out_buf[(i * 2) + 1] = (short)(mix_r[i] * 32767);
	}
	if (!ao_play(device, (void*)out_buf, K_AO_BUF_SIZE * 2 * 2))
	{
		return -1;
	}
	return 0;
}


