

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
#include <stdbool.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_order.h"

#include <Order.h>


static bool order_info(Listener* lr,
		int32_t song_id,
		int32_t subsong,
		Order* order);


int Listener_get_orders(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Order* order = Song_get_order(song);
	assert(order != NULL);
	for (uint16_t i = 0; i < SUBSONGS_MAX; ++i)
	{
		if (Order_is_empty(order, i))
		{
			continue;
		}
		if (!order_info(lr, player_id, i, order))
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
	}
	return 0;
}


int Listener_set_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Order* order = Song_get_order(song);
	assert(order != NULL);
	int32_t subsong = argv[1]->i;
	if (subsong < 0 || subsong >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	int32_t order_index = argv[2]->i;
	if (order_index < 0 || order_index >= ORDERS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid order number");
		return 0;
	}
	int32_t pattern = argv[3]->i;
	if ((pattern < 0 || pattern >= PATTERNS_MAX) && pattern != ORDER_NONE)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid Pattern number");
		return 0;
	}
	if (!Order_set(order, subsong, order_index, pattern))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for new order");
		return 0;
	}
	if (!order_info(lr, player_id, subsong, order))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_ins_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Order* order = Song_get_order(song);
	assert(order != NULL);
	int32_t subsong = argv[1]->i;
	if (subsong < 0 || subsong >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	int32_t order_index = argv[2]->i;
	if (order_index < 0 || order_index >= ORDERS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid order number");
		return 0;
	}
	for (int32_t i = ORDERS_MAX - 2; i >= order_index; --i)
	{
		int16_t pat = Order_get(order, subsong, i);
		if (!Order_set(order, subsong, i + 1, pat))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for new order");
			return 0;
		}
	}
	if (!Order_set(order, subsong, order_index, ORDER_NONE))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for new order");
		return 0;
	}
	if (!order_info(lr, player_id, subsong, order))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_del_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t player_id = argv[0]->i;
	Player* player = lr->player_cur;
	if (player == NULL || player->id != player_id)
	{
		player = Playlist_get(lr->playlist, player_id);
	}
	if (player == NULL)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Song doesn't exist");
		return 0;
	}
	Song* song = player->song;
	Order* order = Song_get_order(song);
	assert(order != NULL);
	int32_t subsong = argv[1]->i;
	if (subsong < 0 || subsong >= SUBSONGS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid subsong number");
		return 0;
	}
	int32_t order_index = argv[2]->i;
	if (order_index < 0 || order_index >= ORDERS_MAX)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Invalid order number");
		return 0;
	}
	for (int32_t i = order_index; i < ORDERS_MAX - 2; ++i)
	{
		int32_t pat = Order_get(order, subsong, i + 1);
		if (!Order_set(order, subsong, i, pat))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for new order");
			return 0;
		}
	}
	if (!Order_set(order, subsong, ORDERS_MAX - 1, ORDER_NONE))
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host, lr->method_path, "s", "Couldn't allocate memory for new order");
		return 0;
	}
	if (!order_info(lr, player_id, subsong, order))
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


static bool order_info(Listener* lr,
		int32_t song_id,
		int32_t subsong,
		Order* order)
{
	assert(lr != NULL);
	assert(subsong >= 0);
	assert(subsong < SUBSONGS_MAX);
	assert(order != NULL);
	lo_message m = lo_message_new();
	lo_message_add_int32(m, song_id);
	lo_message_add_int32(m, subsong);
	uint16_t to_be_added = 0;
	for (uint16_t i = 0; i < ORDERS_MAX; ++i)
	{
		int16_t pat = Order_get(order, subsong, i);
		if (pat != ORDER_NONE)
		{
			for (uint16_t k = to_be_added; k < i; ++k)
			{
				lo_message_add_int32(m, k);
				lo_message_add_int32(m, ORDER_NONE);
			}
			lo_message_add_int32(m, i);
			lo_message_add_int32(m, pat);
			to_be_added = i + 1;
		}
	}
	strcpy(lr->method_path + lr->host_path_len, "order_info");
	int ret = lo_send_message(lr->host, lr->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		return false;
	}
	return true;
}


