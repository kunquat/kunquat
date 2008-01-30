

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <Playlist.h>

#include "Listener.h"

#ifdef ENABLE_JACK
#include <Driver_jack.h>
#endif


typedef struct Driver_info
{
	char* name;
	bool (*init)(Playlist* playlist, uint32_t* freq);
} Driver_info;

static Driver_info drivers[] =
{
#ifdef ENABLE_JACK
	{ "JACK", Driver_jack_init },
#endif
	{ NULL, NULL }
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
	if (l->host == NULL || l->host_path == NULL)
	{
		return 0;
	}
	lo_message m = lo_message_new();
	for (int i = 0; drivers[i].name != NULL; ++i)
	{
		lo_message_add_string(m, drivers[i].name);
	}
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "drivers");
	int ret = lo_send_message(l->host, full_path, m);
	lo_message_free(m);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send driver information\n");
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
	int32_t driver_id = argv[0]->i;
	if (driver_id < 0 ||
			driver_id >= (int32_t)(sizeof(drivers) / sizeof(Driver_info)) - 1)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "error");
		lo_send(l->host,
				full_path,
				"si",
				"Invalid driver ID requested:",
				driver_id);
		xfree(full_path);
		return 0;
	}
	assert(drivers[driver_id].name != NULL);
	assert(drivers[driver_id].init != NULL);
	l->freq = 0;
	if (!drivers[driver_id].init(l->playlist, &l->freq))
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "error");
		lo_send(l->host,
				full_path,
				"s",
				"Couldn't initialise the sound driver");
		xfree(full_path);
		return 0;
	}
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
	int ret = lo_send(l->host, full_path, "i", (int32_t)l->freq);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	return 0;
}


