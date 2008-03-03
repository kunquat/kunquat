

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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <Playlist.h>

#include "Listener.h"
#include "Listener_driver.h"

#ifdef ENABLE_ALSA
#include <Driver_alsa.h>
#endif
#ifdef ENABLE_JACK
#include <Driver_jack.h>
#endif


typedef struct Driver_info
{
	char* name;
	bool (*init)(Playlist* playlist, uint32_t* freq);
	void (*close)(void);
} Driver_info;

static Driver_info drivers[] =
{
#ifdef ENABLE_ALSA
	{ "ALSA", Driver_alsa_init, Driver_alsa_close },
#endif
#ifdef ENABLE_JACK
	{ "JACK", Driver_jack_init, Driver_jack_close },
#endif
	{ NULL, NULL, NULL }
};


int Listener_get_drivers(const char* path,
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
	assert(l->method_path != NULL);
	lo_message m = lo_message_new();
	if (m == NULL)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	for (int i = 0; drivers[i].name != NULL; ++i)
	{
		lo_message_add_string(m, drivers[i].name);
	}
	strcpy(l->method_path + l->host_path_len, "drivers");
	int ret = lo_send_message(l->host, l->method_path, m);
	lo_message_free(m);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send driver information\n");
		return 0;
	}
	return 0;
}


int Listener_active_driver(const char* path,
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
	assert(l->method_path != NULL);
	strcpy(l->method_path + l->host_path_len, "active_driver");
	int ret = lo_send(l->host, l->method_path, "ii",
			(int32_t)l->driver_id,
			(int32_t)l->freq);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_driver_init(const char* path,
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
	if (l->host == NULL)
	{
		return 0;
	}
	assert(l->method_path != NULL);
	int32_t driver_id = argv[0]->i;
	if (driver_id < 0 ||
			driver_id >= (int32_t)(sizeof(drivers) / sizeof(Driver_info)) - 1)
	{
		strcpy(l->method_path + l->host_path_len, "error");
		lo_send(l->host,
				l->method_path,
				"si",
				"Invalid driver ID requested:",
				driver_id);
		return 0;
	}
	assert(drivers[driver_id].name != NULL);
	assert(drivers[driver_id].init != NULL);
	if (l->driver_id >= 0)
	{
		drivers[l->driver_id].close();
	}
	l->freq = 0;
	if (!drivers[driver_id].init(l->playlist, &l->freq))
	{
		strcpy(l->method_path + l->host_path_len, "driver_init");
		lo_send(l->host,
				l->method_path,
				"ss",
				"Error:",
				"Couldn't initialise the sound driver");
		return 0;
	}
	l->driver_id = driver_id;
	strcpy(l->method_path + l->host_path_len, "driver_init");
	int ret = lo_send(l->host, l->method_path, "ii",
			(int32_t)l->driver_id,
			(int32_t)l->freq);
	if (ret == -1)
	{
		fprintf(stderr, "Couldn't send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_driver_close(const char* path,
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
	assert(l->method_path != NULL);
	if (l->driver_id < 0)
	{
		strcpy(l->method_path + l->host_path_len, "notify");
		lo_send(l->host, l->method_path, "s", "No active sound drivers");
		return 0;
	}
	assert(l->driver_id < (int32_t)(sizeof(drivers) / sizeof(Driver_info)) - 1);
	drivers[l->driver_id].close();
	int driver_id = l->driver_id;
	l->driver_id = -1;
	l->freq = 0;
	strcpy(l->method_path + l->host_path_len, "notify");
	int ret = lo_send(l->host, l->method_path, "si",
			"Closed driver", (int32_t)driver_id);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	sleep(1); // Used to prevent another driver for initialising too early.
	return 0;
}


