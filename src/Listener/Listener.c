

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
#include "Listener_ins_pcm.h"
#include "Listener_pattern.h"
#include "Listener_order.h"
#include "Listener_note_table.h"
#include "Listener_player.h"
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
    { "/kunquat/get_song_info", "i", Listener_get_song_info },
    { "/kunquat/set_song_title", "is", Listener_set_song_title },
    { "/kunquat/set_mix_vol", "id", Listener_set_mix_vol },
    { "/kunquat/set_subsong_tempo", "iid", Listener_set_subsong_tempo },
    { "/kunquat/set_subsong_global_vol", "iid", Listener_set_subsong_global_vol },
    { "/kunquat/del_song", "i", Listener_del_song },
    { "/kunquat/get_insts", "i", Listener_get_insts },
    { "/kunquat/new_ins", "iii", Listener_new_ins },
    { "/kunquat/ins_set_name", "iis", Listener_ins_set_name },
//  { "/kunquat/ins_pcm_get_sample_info", TODO },
    { "/kunquat/ins_pcm_load_sample", "iiis", Listener_ins_pcm_load_sample },
    { "/kunquat/ins_pcm_sample_set_mid_freq", "iiid", Listener_ins_pcm_sample_set_mid_freq },
    { "/kunquat/ins_pcm_remove_sample", "iii", Listener_ins_pcm_remove_sample },
    { "/kunquat/ins_pcm_set_mapping", "iiiiidiidd", Listener_ins_pcm_set_mapping },
    { "/kunquat/ins_pcm_del_mapping", "iiiiidi", Listener_ins_pcm_del_mapping },
    { "/kunquat/del_ins", "ii", Listener_del_ins },
    { "/kunquat/get_pattern", "ii", Listener_get_pattern },
    { "/kunquat/set_pat_len", "iihi", Listener_set_pat_len },
    { "/kunquat/pat_ins_event", NULL, Listener_pat_ins_event },
    { "/kunquat/pat_mod_event", NULL, Listener_pat_mod_event },
    { "/kunquat/pat_del_event", "iiihii", Listener_pat_del_event },
    { "/kunquat/pat_del_row", "iiihi", Listener_pat_del_row },
    { "/kunquat/pat_shift_up", "iiihihi", Listener_pat_shift_up },
    { "/kunquat/pat_shift_down", "iiihihi", Listener_pat_shift_down },
    { "/kunquat/get_orders", "i", Listener_get_orders },
    { "/kunquat/set_order", "iiii", Listener_set_order },
    { "/kunquat/ins_order", "iii", Listener_ins_order },
    { "/kunquat/del_order", "iii", Listener_del_order },
    { "/kunquat/get_note_table", "ii", Listener_get_note_table },
    { "/kunquat/set_note_table_name", "iis", Listener_set_note_table_name },
    { "/kunquat/set_note_table_ref_note", "iii", Listener_set_note_table_ref_note },
    { "/kunquat/set_note_table_ref_pitch", "iid", Listener_set_note_table_ref_pitch },
    { "/kunquat/set_note_table_octave_ratio", NULL, Listener_set_note_table_octave_ratio },
    { "/kunquat/set_note_name", "iiis", Listener_set_note_name },
    { "/kunquat/set_note_ratio", NULL, Listener_set_note_ratio },
    { "/kunquat/ins_note", "iii", Listener_ins_note },
    { "/kunquat/del_note", "iii", Listener_del_note },
    { "/kunquat/del_note_table", "ii", Listener_del_note_table },
    { "/kunquat/stop", "", Listener_stop },
    { "/kunquat/stop_song", "i", Listener_stop_song },
    { "/kunquat/play_song", "i", Listener_play_song },
    { "/kunquat/play_subsong", "ii", Listener_play_subsong },
    { "/kunquat/play_pattern", "iid", Listener_play_pattern },
    { "/kunquat/play_event", NULL, Listener_play_event },
    { "/kunquat/play_stats", "", Listener_play_stats },
    { "/kunquat/demo", "", Listener_demo },
    { NULL, NULL, Listener_fallback },
    { NULL, NULL, NULL }
};


/**
 * Initialises a Listener.
 *
 * \param lr   The Listener -- must not be \c NULL.
 *
 * \return   The parameter \a lr if successful, or \c NULL if failed.
 *           A failure occurs if memory allocation fails, the server cannot be
 *           initialised or the server cannot be monitored using select().
 */
Listener* Listener_init(Listener* lr);


/**
 * Uninitalises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 */
void Listener_uninit(Listener* lr);


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
    Listener* lr = Listener_init(&(Listener){ .done = false });
    if (lr == NULL)
    {
        exit(LISTENER_ERROR_CREATE);
    }
    do
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(lr->lo_fd, &rfds);
        int ret = select(lr->lo_fd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1)
        {
            Listener_uninit(lr);
            fprintf(stderr, "select() error\n");
            exit(LISTENER_ERROR_SELECT);
        }
        else if (ret > 0)
        {
            if (FD_ISSET(lr->lo_fd, &rfds))
            {
                lo_server_recv_noblock(lr->s, 0);
            }
        }
    } while (!lr->done);
    Listener_uninit(lr);
    exit(LISTENER_ERROR_NONE);
}


Listener* Listener_init(Listener* lr)
{
    assert(lr != NULL);
    lr->done = false;

    lr->playlist = new_Playlist();
    if (lr->playlist == NULL)
    {
        return NULL;
    }

    lr->s = lo_server_new_with_proto("7770", LO_UDP, Listener_error);
    if (lr->s == NULL)
    {
        del_Playlist(lr->playlist);
        return NULL;
    }
    lr->lo_fd = lo_server_get_socket_fd(lr->s);
    if (lr->lo_fd < 0)
    {
        lo_server_free(lr->s);
        del_Playlist(lr->playlist);
        return NULL;
    }
    lr->host = NULL;
    lr->host_hostname = NULL;
    lr->host_port = NULL;
    lr->host_path = NULL;
    lr->host_path_len = 0;
    lr->method_path = NULL;

    lr->driver_id = -1;

    lr->voice_count = 64;
    lr->player_cur = NULL;
    lr->freq = 1;

    for (int i = 0; methods[i].handler != NULL; ++i)
    {
        if (lo_server_add_method(lr->s,
                methods[i].path,
                methods[i].format,
                methods[i].handler, lr) == NULL)
        {
            lo_server_free(lr->s);
            del_Playlist(lr->playlist);
            return NULL;
        }
    }

    return lr;
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
    Listener* lr = user_data;
    if (lr->host != NULL)
    {
        lo_address_free(lr->host);
        lr->host = NULL;
    }
    if (lr->host_hostname != NULL)
    {
        free(lr->host_hostname);
        lr->host_hostname = NULL;
    }
    if (lr->host_port != NULL)
    {
        free(lr->host_port);
        lr->host_port = NULL;
    }
    if (lr->host_path != NULL)
    {
        free(lr->host_path);
        lr->host_path = NULL;
    }
    char* url = &argv[0]->s;
    lr->host_hostname = lo_url_get_hostname(url);
    if (lr->host_hostname == NULL)
    {
        fprintf(stderr, "Couldn't parse the host URL\n");
        goto cleanup;
    }
    lr->host_port = lo_url_get_port(url);
    if (lr->host_port == NULL)
    {
        fprintf(stderr, "Couldn't parse the host URL\n");
        goto cleanup;
    }
    lr->host_path = lo_url_get_path(url);
    if (lr->host_path == NULL)
    {
        fprintf(stderr, "Couldn't parse the host URL\n");
        goto cleanup;
    }
    lr->host_path_len = strlen(lr->host_path);
    lr->host = lo_address_new(lr->host_hostname, lr->host_port);
    if (lr->host == NULL)
    {
        fprintf(stderr, "Couldn't create an address object\n");
        goto cleanup;
    }
    lr->method_path = xcalloc(char, lr->host_path_len + METHOD_NAME_MAX);
    if (lr->method_path == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory\n");
        goto cleanup;
    }
    strcpy(lr->method_path, lr->host_path);
    lo_message m = lo_message_new();
    if (m == NULL)
    {
        msg_alloc_fail();
        goto cleanup;
    }
    lo_message_add_string(m, "Hello");
    int ret = 0;
    send_msg(lr, "notify", m, ret);
    lo_message_free(m);
    if (ret == -1)
    {
        goto cleanup;
    }
    return 0;

cleanup:

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
        lr->host_path_len = 0;
    }
    if (lr->method_path != NULL)
    {
        xfree(lr->method_path);
        lr->method_path = NULL;
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
    Listener* lr = user_data;
    if (lr->host == NULL || lr->host_path == NULL)
    {
        return 0;
    }
    lo_message m = new_msg();
    char ver_str[32] = { '\0' };
    snprintf(ver_str, 32, "%d.%d.%d",
            KUNQUAT_VERSION_MAJOR,
            KUNQUAT_VERSION_MINOR,
            KUNQUAT_VERSION_PATCH);
    lo_message_add_string(m, ver_str);
    lo_message_add_string(m, BUILD_TIME);
    int ret = 0;
    send_msg(lr, "version", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_quit(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
/*  (void)path;
    (void)types;
    (void)argv;
    (void)argc;
    (void)msg; */
    assert(user_data != NULL);
    Listener* lr = user_data;
    Player* player = lr->playlist->first;
    while (player != NULL)
    {
        Player_stop(player);
        player = player->next;
    }
    if (lr->driver_id >= 0)
    {
        Listener_driver_close(path, types, argv, argc, msg, lr);
    }
    if (lr->host != NULL)
    {
        assert(lr->method_path != NULL);
        lo_message m = lo_message_new();
        if (m != NULL)
        {
            lo_message_add_string(m, "Bye");
            int ret = 0;
            send_msg(lr, "notify", m, ret);
            lo_message_free(m);
        }
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    lo_message m = new_msg();
    for (int i = 0; methods[i].path != NULL; ++i)
    {
        lo_message_add_string(m, methods[i].path);
    }
    int ret = 0;
    send_msg(lr, "notify", m, ret);
    lo_message_free(m);
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
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    lo_message m = new_msg();
    lo_message_add_string(m, "Unrecognised command:");
    lo_message_add_string(m, path);
    lo_message_add_string(m, types);
    int ret = 0;
    send_msg(lr, "notify", m, ret);
    lo_message_free(m);
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
    check_cond(lr, voices > 0 && voices <= MAX_VOICES,
            "The number of Voices (%ld)", (long)voices);
    lr->voice_count = (uint16_t)voices;
    Player* player = lr->playlist->first;
    while (player != NULL)
    {
        if (!Voice_pool_resize(player->voices, voices))
        {
            send_memory_fail(lr, "the Voices of the Song");
        }
        player = player->next;
    }
    lo_message m = new_msg();
    lo_message_add_string(m, "Allocated");
    lo_message_add_int32(m, voices);
    if (voices == 1)
    {
        lo_message_add_string(m, "Voice");
    }
    else
    {
        lo_message_add_string(m, "Voices");
    }
    int ret = 0;
    send_msg(lr, "notify", m, ret);
    lo_message_free(m);
    return 0;
}


