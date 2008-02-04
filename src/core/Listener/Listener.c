

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

#include <xmemory.h>


#define KUNQUAT_VERSION_MAJOR (0)
#define KUNQUAT_VERSION_MINOR (0)
#define KUNQUAT_VERSION_PATCH (0)


/**
 * Initialises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 *
 * \return   The parameter \a l if successful, or \c NULL if failed.
 *           A failure occurs if the server cannot be initialised or the
 *           server cannot be monitored using select().
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

	l->driver_id = -1;

	l->voices = NULL;
	l->freq = 0;

	lo_server_add_method(l->s, "/kunquat/quit", "", Listener_quit, l);
	lo_server_add_method(l->s, "/kunquat/register_host", "s", Listener_register_host, l);
	lo_server_add_method(l->s, "/kunquat/version", "", Listener_version, l);

	lo_server_add_method(l->s, "/kunquat/get_drivers", "", Listener_get_drivers, l);
	lo_server_add_method(l->s, "/kunquat/driver_init", "i", Listener_driver_init, l);
	lo_server_add_method(l->s, "/kunquat/driver_close", "", Listener_driver_close, l);

	lo_server_add_method(l->s, "/kunquat/set_voices", "i", Listener_set_voices, l);

	lo_server_add_method(l->s, NULL, NULL, Listener_fallback, l);
	return l;
}


void Listener_uninit(Listener* l)
{
	assert(l != NULL);
	// TODO: Close sound driver if needed
	lo_server_free(l->s);
	if (l->host != NULL)
	{
		lo_address_free(l->host);
	}
	if (l->host_hostname != NULL)
	{
		free(l->host_hostname);
	}
	if (l->host_port != NULL)
	{
		free(l->host_port);
	}
	if (l->host_path != NULL)
	{
		free(l->host_path);
	}
	if (l->voices != NULL)
	{
		del_Voice_pool(l->voices);
	}
	del_Playlist(l->playlist);
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
	(void)msg;
	assert(user_data != NULL);
	if (argc < 1)
	{
		fprintf(stderr, "Too few arguments to register_host (expected host URL)\n");
		return 0;
	}
	Listener* l = user_data;
	if (l->host != NULL)
	{
		lo_address_free(l->host);
	}
	if (l->host_hostname != NULL)
	{
		free(l->host_hostname);
	}
	if (l->host_port != NULL)
	{
		free(l->host_port);
	}
	if (l->host_path != NULL)
	{
		free(l->host_path);
	}
	char* url = &argv[0]->s;
	l->host_hostname = lo_url_get_hostname(url);
	l->host_port = lo_url_get_port(url);
	l->host_path = lo_url_get_path(url);
	l->host = lo_address_new(l->host_hostname, l->host_port);
	if (l->host == NULL)
	{
		fprintf(stderr, "Failed to create an address object\n");
		return 0;
	}
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
	int ret = lo_send(l->host, full_path, "s", "Hello");
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the confirmation message\n");
		return 0;
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
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "version");
	int ret = lo_send(l->host,
			full_path,
			"iii",
			(int32_t)KUNQUAT_VERSION_MAJOR,
			(int32_t)KUNQUAT_VERSION_MINOR,
			(int32_t)KUNQUAT_VERSION_PATCH);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send version information\n");
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
	(void)path;
	(void)types;
	(void)argv;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host != NULL && l->host_path != NULL)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
		lo_send(l->host, full_path, "s", "Bye");
		xfree(full_path);
	}
	l->done = true;
	return 0;
}


int Listener_fallback(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
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
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
	int ret = lo_send(l->host, full_path, "ss", "Unrecognised command:", path);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
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
	Listener* l = user_data;
	if (l->host == NULL || l->host_path == NULL || l->voices != NULL) // FIXME
	{
		return 0;
	}
	int32_t voices = argv[0]->i;
	if (voices <= 0 || voices > MAX_VOICES)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "error");
		lo_send(l->host,
				full_path,
				"si",
				"Invalid number of Voices requested:",
				voices);
		xfree(full_path);
		return 0;
	}
	l->voices = new_Voice_pool(voices, 32);
	if (l->voices == NULL)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "error");
		lo_send(l->host,
				full_path,
				"s",
				"Couldn't allocate memory for Voices");
		xfree(full_path);
		return 0;
	}
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
	char out_msg[] = "Allocated XXXXXXXXXXX Voices";
	char* pos = strstr(out_msg, "XXX");
	assert(pos != NULL);
	sprintf(pos, "%hu Voices", (unsigned short)voices);
	int ret = lo_send(l->host, full_path, "s", out_msg);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	return 0;
}


