

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "lo/lo.h"

#include "Listener.h"
#include "Listener_driver.h"
#include "Listener_song.h"
#include "Listener_ins.h"
#include "Listener_pattern.h"
#include "Listener_note_table.h"
#include "Listener_demo.h"

#include <xmemory.h>


#define KUNQUAT_VERSION_MAJOR (0)
#define KUNQUAT_VERSION_MINOR (0)
#define KUNQUAT_VERSION_PATCH (0)


typedef struct Method_desc
{
	char* path;
	char* format;
	lo_method_handler handler;
} Method_desc;

static Method_desc methods[] =
{
	{ "/kunquat/quit", "", Listener_quit },
	{ "/kunquat/help", "", Listener_help },
	{ "/kunquat/register_host", "s", Listener_register_host },
	{ "/kunquat/version", "", Listener_version },
	{ "/kunquat/get_drivers", "", Listener_get_drivers },
	{ "/kunquat/active_driver", "", Listener_active_driver },
	{ "/kunquat/driver_init", "i", Listener_driver_init },
	{ "/kunquat/driver_close", "", Listener_driver_close },
	{ "/kunquat/set_voices", "i", Listener_set_voices },
	{ "/kunquat/new_song", "", Listener_new_song },
	{ "/kunquat/get_songs", "", Listener_get_songs },
	{ "/kunquat/del_song", "i", Listener_del_song },
	{ "/kunquat/get_insts", "i", Listener_get_insts },
	{ "/kunquat/new_ins", "iii", Listener_new_ins },
	{ "/kunquat/ins_set_name", "iis", Listener_ins_set_name },
	{ "/kunquat/del_ins", "ii", Listener_del_ins },
	{ "/kunquat/get_pattern", "ii", Listener_get_pattern },
	{ "/kunquat/pat_ins_event", NULL, Listener_pat_ins_event },
	{ "/kunquat/get_note_table", "i", Listener_get_note_table },
	{ "/kunquat/demo", "", Listener_demo },
	{ NULL, NULL, Listener_fallback },
	{ NULL, NULL, NULL }
};


/**
 * Initialises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 *
 * \return   The parameter \a l if successful, or \c NULL if failed.
 *           A failure occurs if memory allocation fails, the server cannot be
 *           initialised or the server cannot be monitored using select().
 */
Listener* Listener_init(Listener* l);


/**
 * Uninitalises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 */
void Listener_uninit(Listener* l);


/**
 * The error handler for the server.
 *
 * \param num     The error identification.
 * \param msg     A textual description of the error.
 * \param where   The location of the error.
 */
void Listener_error(int num, const char* msg, const char* where);


int main(void)
{
	Listener* l = Listener_init(&(Listener){ .done = false });
	if (l == NULL)
	{
		exit(LISTENER_ERROR_CREATE);
	}
	do
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(l->lo_fd, &rfds);
		int ret = select(l->lo_fd + 1, &rfds, NULL, NULL, NULL);
		if (ret == -1)
		{
			Listener_uninit(l);
			fprintf(stderr, "select() error\n");
			exit(LISTENER_ERROR_SELECT);
		}
		else if (ret > 0)
		{
			if (FD_ISSET(l->lo_fd, &rfds))
			{
				lo_server_recv_noblock(l->s, 0);
			}
		}
	} while (!l->done);
	Listener_uninit(l);
	exit(LISTENER_ERROR_NONE);
}


Listener* Listener_init(Listener* l)
{
	assert(l != NULL);
	l->done = false;

	l->playlist = new_Playlist();
	if (l->playlist == NULL)
	{
		return NULL;
	}

	l->s = lo_server_new("7770", Listener_error);
	if (l->s == NULL)
	{
		del_Playlist(l->playlist);
		return NULL;
	}
	l->lo_fd = lo_server_get_socket_fd(l->s);
	if (l->lo_fd < 0)
	{
		lo_server_free(l->s);
		del_Playlist(l->playlist);
		return NULL;
	}
	l->host = NULL;
	l->host_hostname = NULL;
	l->host_port = NULL;
	l->host_path = NULL;
	l->host_path_len = 0;
	l->method_path = NULL;

	l->driver_id = -1;

	l->voice_count = 64;
	l->player_cur = NULL;
	l->freq = 1;

	for (int i = 0; methods[i].handler != NULL; ++i)
	{
		if (lo_server_add_method(l->s,
				methods[i].path,
				methods[i].format,
				methods[i].handler, l) == NULL)
		{
			lo_server_free(l->s);
			del_Playlist(l->playlist);
			return NULL;
		}
	}

	return l;
}


void Listener_uninit(Listener* lr)
{
	assert(lr != NULL);
	// TODO: Close sound driver if needed
	lo_server_free(lr->s);
	if (lr->host != NULL)
	{
		lo_address_free(lr->host);
		lr->host = NULL;
	}
	if (lr->host_hostname != NULL)
	{
		xfree(lr->host_hostname);
		lr->host_hostname = NULL;
	}
	if (lr->host_port != NULL)
	{
		xfree(lr->host_port);
		lr->host_port = NULL;
	}
	if (lr->host_path != NULL)
	{
		xfree(lr->host_path);
		lr->host_path = NULL;
	}
	if (lr->method_path != NULL)
	{
		xfree(lr->method_path);
		lr->method_path = NULL;
	}
	del_Playlist(lr->playlist);
	lr->playlist = NULL;
/*	if (lr->voices != NULL)
	{
		del_Voice_pool(lr->voices);
		lr->voices = NULL;
	} */
	return;
}


void Listener_error(int num, const char* msg, const char* where)
{
	fprintf(stderr, "liblo server error %d in %s: %s\n", num, where, msg);
	return;
}


int Listener_register_host(const char* path,
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
	Listener* l = user_data;
	if (l->host != NULL)
	{
		lo_address_free(l->host);
		l->host = NULL;
	}
	if (l->host_hostname != NULL)
	{
		free(l->host_hostname);
		l->host_hostname = NULL;
	}
	if (l->host_port != NULL)
	{
		free(l->host_port);
		l->host_port = NULL;
	}
	if (l->host_path != NULL)
	{
		free(l->host_path);
		l->host_path = NULL;
	}
	char* url = &argv[0]->s;
	l->host_hostname = lo_url_get_hostname(url);
	if (l->host_hostname == NULL)
	{
		fprintf(stderr, "Couldn't parse the host URL\n");
		goto cleanup;
	}
	l->host_port = lo_url_get_port(url);
	if (l->host_port == NULL)
	{
		fprintf(stderr, "Couldn't parse the host URL\n");
		goto cleanup;
	}
	l->host_path = lo_url_get_path(url);
	if (l->host_path == NULL)
	{
		fprintf(stderr, "Couldn't parse the host URL\n");
		goto cleanup;
	}
	l->host_path_len = strlen(l->host_path);
	l->host = lo_address_new(l->host_hostname, l->host_port);
	if (l->host == NULL)
	{
		fprintf(stderr, "Couldn't create an address object\n");
		goto cleanup;
	}
	l->method_path = xcalloc(char, l->host_path_len + METHOD_NAME_MAX);
	if (l->method_path == NULL)
	{
		fprintf(stderr, "Couldn't allocate memory\n");
		goto cleanup;
	}
	strcpy(l->method_path, l->host_path);
	strcpy(l->method_path + l->host_path_len, "notify");
	int ret = lo_send(l->host, l->method_path, "s", "Hello");
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the confirmation message\n");
		goto cleanup;
	}
	return 0;

cleanup:

	if (l->host != NULL)
	{
		lo_address_free(l->host);
		l->host = NULL;
	}
	if (l->host_hostname != NULL)
	{
		xfree(l->host_hostname);
		l->host_hostname = NULL;
	}
	if (l->host_port != NULL)
	{
		xfree(l->host_port);
		l->host_port = NULL;
	}
	if (l->host_path != NULL)
	{
		xfree(l->host_path);
		l->host_path = NULL;
		l->host_path_len = 0;
	}
	if (l->method_path != NULL)
	{
		xfree(l->method_path);
		l->method_path = NULL;
	}
	return 0;
}


int Listener_version(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argv;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL || l->host_path == NULL)
	{
		return 0;
	}
	strcpy(l->method_path + l->host_path_len, "version");
	int ret = lo_send(l->host,
			l->method_path,
			"iii",
			(int32_t)KUNQUAT_VERSION_MAJOR,
			(int32_t)KUNQUAT_VERSION_MINOR,
			(int32_t)KUNQUAT_VERSION_PATCH);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send version information\n");
		return 0;
	}
	return 0;
}


int Listener_quit(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
/*	(void)path;
	(void)types;
	(void)argv;
	(void)argc;
	(void)msg; */
	assert(user_data != NULL);
	Listener* lr = user_data;
	Player* player = lr->playlist->first;
	while (player != NULL)
	{
		Player_set_state(player, STOP);
		player = player->next;
	}
	if (lr->driver_id >= 0)
	{
		Listener_driver_close(path, types, argv, argc, msg, lr);
	}
	if (lr->host != NULL)
	{
		assert(lr->method_path != NULL);
		strcpy(lr->method_path + lr->host_path_len, "notify");
		lo_send(lr->host, lr->method_path, "s", "Bye");
	}
	lr->done = true;
	return 0;
}


int Listener_help(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argv;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	lo_message m = lo_message_new();
	for (int i = 0; methods[i].path != NULL; ++i)
	{
		lo_message_add_string(m, methods[i].path);
	}
	strcpy(l->method_path + l->host_path_len, "notify");
	int ret = lo_send_message(l->host, l->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send help\n");
		return 0;
	}
	return 0;
}


int Listener_fallback(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)argv;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	strcpy(l->method_path + l->host_path_len, "notify");
	int ret = lo_send(l->host, l->method_path, "sss", "Unrecognised command:", path, types);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_set_voices(const char* path,
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
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t voices = argv[0]->i;
	if (voices <= 0 || voices > MAX_VOICES)
	{
		strcpy(lr->method_path + lr->host_path_len, "error");
		lo_send(lr->host,
				lr->method_path,
				"si",
				"Invalid number of Voices requested:",
				voices);
		return 0;
	}
	lr->voice_count = (uint16_t)voices;
	Player* player = lr->playlist->first;
	int32_t fail_count = 0;
	while (player != NULL)
	{
		if (!Voice_pool_resize(player->voices, voices))
		{
			strcpy(lr->method_path + lr->host_path_len, "error");
			lo_send(lr->host,
					lr->method_path,
					"si",
					"Couldn't allocate memory for Voices of Song",
					player->id);
			++fail_count;
		}
		player = player->next;
	}
/*	if (l->voices == NULL)
	{
		l->voices = new_Voice_pool(voices, 32);
		if (l->voices == NULL)
		{
			strcpy(l->method_path + l->host_path_len, "error");
			lo_send(l->host,
					l->method_path,
					"s",
					"Couldn't allocate memory for Voices");
			return 0;
		}
	}
	else if (Voice_pool_get_size(l->voices) != voices)
	{
		if (!Voice_pool_resize(l->voices, voices))
		{
			strcpy(l->method_path + l->host_path_len, "error");
			lo_send(l->host,
					l->method_path,
					"s",
					"Couldn't allocate memory for Voices");
		}
	}
	voices = Voice_pool_get_size(l->voices); */
	if (fail_count == 0)
	{
		strcpy(lr->method_path + lr->host_path_len, "notify");
		int ret = lo_send(lr->host, lr->method_path, "sis",
				"Allocated", (int32_t)voices, "Voices");
		if (ret == -1)
		{
			fprintf(stderr, "Couldn't send the response message\n");
			return 0;
		}
	}
	return 0;
}


