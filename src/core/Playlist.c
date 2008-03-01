

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

#include "Playlist.h"

#include <xmemory.h>


Playlist* new_Playlist(void)
{
	Playlist* playlist = xalloc(Playlist);
	if (playlist == NULL)
	{
		return NULL;
	}
	playlist->first = NULL;
	return playlist;
}


void Playlist_ins(Playlist* playlist, Player* player)
{
	assert(playlist != NULL);
	assert(player != NULL);
	assert(player->next == NULL);
	assert(player->prev == NULL);
	player->next = playlist->first;
	if (playlist->first != NULL)
	{
		playlist->first->prev = player;
	}
	player->prev = NULL;
	playlist->first = player;
	return;
}


Player* Playlist_get(Playlist* playlist, int32_t id)
{
	assert(playlist != NULL);
	Player* cur = playlist->first;
	while (cur != NULL && cur->id != id)
	{
		cur = cur->next;
	}
	return cur;
}


void Playlist_remove(Playlist* playlist, Player* player)
{
	assert(playlist != NULL);
	assert(player != NULL);
	Player* next = player->next;
	Player* prev = player->prev;
	if (next != NULL)
	{
		next->prev = prev;
	}
	if (prev != NULL)
	{
		prev->next = next;
	}
	if (player == playlist->first)
	{
		assert(prev == NULL);
		playlist->first = next;
	}
	del_Player(player);
	return;
}


void del_Playlist(Playlist* playlist)
{
	assert(playlist != NULL);
	Player* target = playlist->first;
	playlist->first = NULL;
	while (target != NULL)
	{
		Player* next = target->next;
		del_Player(target);
		target = next;
	}
	xfree(playlist);
	return;
}


