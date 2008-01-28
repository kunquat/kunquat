

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

#include "Player.h"

#include <xmemory.h>


Player* new_Player(uint32_t freq, Voice_pool* pool, Song* song)
{
	assert(freq > 0);
	assert(pool != NULL);
	assert(song != NULL);
	Player* player = xalloc(Player);
	if (player == NULL)
	{
		return NULL;
	}
	player->song = song;
	player->play = new_Playdata(freq, pool, Song_get_insts(song));
	if (player->play == NULL)
	{
		xfree(player);
		return NULL;
	}
	return player;
}


uint32_t Player_mix(Player* player, uint32_t nframes)
{
	assert(player != NULL);
	if (!player->play || player->song == NULL)
	{
		return 0;
	}
	int buf_count = Song_get_buf_count(player->song);
	frame_t** bufs = Song_get_bufs(player->song);
	for (int i = 0; i < buf_count; ++i)
	{
		assert(bufs[i] != NULL);
		for (uint32_t k = 0; k < nframes; ++k)
		{
			bufs[i][k] = 0;
		}
	}
	return Song_mix(player->song, nframes, player->play);
}


void Player_set_state(Player* player, Play_mode mode)
{
	assert(player != NULL);
	assert(mode < PLAY_LAST);
	player->play->mode = mode;
	if (!player->play->mode)
	{
		player->play->tempo = Song_get_tempo(player->song);
		player->play->order = 0;
		Reltime_init(&player->play->play_time);
		Reltime_init(&player->play->pos);
		player->play->play_frames = 0;
	}
	return;
}


void del_Player(Player* player)
{
	assert(player != NULL);
	del_Playdata(player->play);
	if (player->song != NULL)
	{
		del_Song(player->song);
	}
	xfree(player);
}


