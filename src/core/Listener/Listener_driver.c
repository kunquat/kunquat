

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
#ifdef ENABLE_AO
#include <Driver_ao.h>
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
#ifdef ENABLE_AO
    { "libao", Driver_ao_init, Driver_ao_close },
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    lo_message m = new_msg();
    for (int i = 0; drivers[i].name != NULL; ++i)
    {
        lo_message_add_string(m, drivers[i].name);
    }
    int ret = 0;
    send_msg(lr, "drivers", m, ret);
    lo_message_free(m);
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    lo_message m = new_msg();
    lo_message_add_int32(m, lr->driver_id);
    lo_message_add_int32(m, lr->freq);
    int ret = 0;
    send_msg(lr, "active_driver", m, ret);
    lo_message_free(m);
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t driver_id = argv[0]->i;
    int32_t driver_count = (int32_t)(sizeof(drivers) / sizeof(Driver_info)) - 1;
    check_cond(lr, driver_id >= 0 && driver_id < driver_count,
            "The driver ID (%ld)", (long)driver_id);
    assert(drivers[driver_id].name != NULL);
    assert(drivers[driver_id].init != NULL);
    if (lr->driver_id >= 0)
    {
        drivers[lr->driver_id].close();
        sleep(1); // Used to prevent another driver from initialising too early.
    }
    lr->freq = 1;
    if (!drivers[driver_id].init(lr->playlist, &lr->freq))
    {
        lo_message m = new_msg();
        lo_message_add_string(m, "Error:");
        lo_message_add_string(m, "Couldn't initialise the sound driver");
        int ret = 0;
        send_msg(lr, "driver_init", m, ret);
        lo_message_free(m);
        return 0;
    }
    assert(lr->freq > 0);
    lr->driver_id = driver_id;
    Playlist_set_mix_freq(lr->playlist, lr->freq);
    lo_message m = new_msg();
    lo_message_add_int32(m, lr->driver_id);
    lo_message_add_int32(m, lr->freq);
    int ret = 0;
    send_msg(lr, "driver_init", m, ret);
    lo_message_free(m);
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    if (lr->driver_id < 0)
    {
        lo_message m = new_msg();
        lo_message_add_string(m, "No active audio drivers");
        int ret = 0;
        send_msg(lr, "notify", m, ret);
        lo_message_free(m);
        return 0;
    }
    assert(lr->driver_id < (int32_t)(sizeof(drivers) / sizeof(Driver_info)) - 1);
    drivers[lr->driver_id].close();
    int driver_id = lr->driver_id;
    lr->driver_id = -1;
    lr->freq = 1;
    lo_message m = new_msg();
    lo_message_add_string(m, "Closed the audio driver:");
    lo_message_add_string(m, drivers[driver_id].name);
    int ret = 0;
    send_msg(lr, "notify", m, ret);
    lo_message_free(m);
    return 0;
}


