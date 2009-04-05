

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


#ifndef K_LISTENER_H
#define K_LISTENER_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <Voice_pool.h>
#include <Playlist.h>
#include "utf8.h"

#include "lo/lo.h"

#include <xmemory.h>


#define METHOD_NAME_MAX (32)


typedef enum
{
    LISTENER_ERROR_NONE = 0,
    LISTENER_ERROR_CREATE,
    LISTENER_ERROR_SELECT
} Listener_err;


typedef struct Listener
{
    /// Used to indicate exit.
    bool done;
    /// The OSC server.
    lo_server s;
    /// The file descriptor of the OSC server socket.
    int lo_fd;
    /// The OSC client address of the host application.
    lo_address host;
    /// The hostname (location) of the host application.
    char* host_hostname;
    /// The port of the host application.
    char* host_port;
    /// The path of the host application.
    char* host_path;
    /// The length of the host path.
    int host_path_len;
    /// The path of the method used.
    char* method_path;

    /// Current sound driver ID. Negative value implies none.
    int driver_id;

    /// Number of Voices.
    uint16_t voice_count;
    /// Playback state information.
    Playlist* playlist;
    /// Player currently in use.
    Player* player_cur;
    /// Mixing frequency.
    uint32_t freq;
} Listener;


typedef int Listener_callback(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data);


#define METHOD_PATH_ALLOC(full, path, method) do\
    {\
        (full) = xnalloc(char, strlen(path) + strlen(method) + 1);\
        if ((full) == NULL)\
        {\
            fprintf(stderr, "Out of memory at %s:%d\n", __FILE__, __LINE__);\
            return 0;\
        }\
        strcpy((full), (path));\
        strcat((full), (method));\
    } while (0)


/**
 * Sends an OSC message and prints a notification on failure.
 *
 * \param lr            The Listener -- must not be \c NULL.
 * \param method_name   The name of the method call -- must not be \c NULL.
 * \param msg           The liblo message object -- must not be \c NULL.
 * \param ret           An integer for storing the return value.
 */
#define send_msg(lr, method_name, msg, ret) do\
    {\
        assert((lr) != NULL);\
        assert((method_name) != NULL);\
        assert((msg) != NULL);\
        strcpy((lr)->method_path + (lr)->host_path_len, (method_name));\
        (ret) = lo_send_message((lr)->host, (lr)->method_path, (msg));\
        if ((ret) == -1)\
        {\
            fprintf(stderr, "Couldn't send the response message at %s:%d\n",\
                    __FILE__, __LINE__);\
        }\
    } while (false)


#define msg_alloc_fail() fprintf(stderr, "Couldn't allocate memory for"\
        " the response message at %s:%d\n", __FILE__, __LINE__)


/**
 * Sends a message reporting that kunquat has run out of memory and gives up.
 *
 * \param lr     The Listener -- must not be \c NULL.
 * \param desc   Textual description of the object for which kunquat couldn't
 *               allocate memory -- must not be \c NULL.
 */
#define send_memory_fail(lr, desc) do\
    {\
        assert((lr) != NULL);\
        assert((desc) != NULL);\
        lo_message mm_ = lo_message_new();\
        if (mm_ == NULL)\
        {\
            msg_alloc_fail();\
            return 0;\
        }\
        lo_message_add_string(mm_, "Couldn't allocate memory for");\
        lo_message_add_string(mm_, (desc));\
        int ret = 0;\
        send_msg((lr), "error", mm_, ret);\
        lo_message_free(mm_);\
        return 0;\
    } while (false)


/**
 * Creates a liblo message. Assumes that the message is stored into the
 * variable \a m. It expands into two statements so it must always be
 * surrounded by braces if used inside a loop or conditional statement.
 */
#define new_msg() lo_message_new(); do\
    {\
        if (m == NULL)\
        {\
            msg_alloc_fail();\
            return 0;\
        }\
    } while (false)


/**
 * Validates the type of an OSC argument.
 *
 * \param lr           The Listener -- must not be \c NULL.
 * \param actual       The actual type descriptor of the value received.
 * \param expected     The expected type descriptor.
 * \param param_name   A human-readable name of the parameter -- must not be
 *                     \c NULL. This is used in the error message.
 */
#define validate_type(lr, actual, expected, param_name) do\
    {\
        assert((lr) != NULL);\
        assert((param_name) != NULL);\
        if ((actual) != (expected))\
        {\
            lo_message msgv_ = lo_message_new();\
            if (msgv_ == NULL)\
            {\
                msg_alloc_fail();\
                return 0;\
            }\
            char err_str_[128] = { '\0' };\
            snprintf(err_str_, 128, "Invalid type of %s: %c (expected %c)",\
                    (param_name), (actual), (expected));\
            lo_message_add_string(msgv_, err_str_);\
            int ret = 0;\
            send_msg((lr), "error", msgv_, ret);\
            lo_message_free(msgv_);\
            return 0;\
        }\
    } while (false)


/**
 * Checks a condition.
 *
 * \param lr       The Listener -- must not be \c NULL.
 * \param cond     The condition (a boolean expression) to be checked.
 * \param format   A human-readable name of the parameter with an initial
 *                 uppercase letter and the formatting of the values -- must
 *                 not be \c NULL. This is used in the error message.
 * \param ...      The checked parameter(s).
 */
#define check_cond(lr, cond, format, ...) do\
    {\
        assert((lr) != NULL);\
        assert((format) != NULL);\
        if (!(cond))\
        {\
            lo_message msgc_ = lo_message_new();\
            if (msgc_ == NULL)\
            {\
                msg_alloc_fail();\
                return 0;\
            }\
            char err_strc_[256] = { '\0' };\
            snprintf(err_strc_, 256, format " does not meet the condition: %s",\
                    __VA_ARGS__, #cond);\
            lo_message_add_string(msgc_, err_strc_);\
            int ret = 0;\
            send_msg((lr), "error", msgc_, ret);\
            lo_message_free(msgc_);\
            return 0;\
        }\
    } while (false)


/**
 * Gets the Player of the Song with the given ID.
 *
 * \param lr        The Listener -- must not be \c NULL.
 * \param song_id   The ID of the Song.
 * \param type      The OSC type description received from the caller.
 */
#define get_player(lr, song_id, type) do\
    {\
        assert((lr) != NULL);\
        validate_type((lr), (type), 'i', "the Song ID");\
        Player* player_ = (lr)->player_cur;\
        if (player_ == NULL || player_->id != (song_id))\
        {\
            player_ = Playlist_get_player((lr)->playlist, (song_id));\
        }\
        if (player_ == NULL)\
        {\
            lo_message msgp_ = lo_message_new();\
            if (msgp_ == NULL)\
            {\
                msg_alloc_fail();\
                return 0;\
            }\
            lo_message_add_string(msgp_, "Song doesn't exist:");\
            lo_message_add_int32(msgp_, (song_id));\
            strcpy((lr)->method_path + (lr)->host_path_len, "error");\
            int ret = 0;\
            send_msg(lr, "error", msgp_, ret);\
            lo_message_free(msgp_);\
            return 0;\
        }\
        (lr)->player_cur = player_;\
        assert(Player_get_id((lr)->player_cur) == (song_id));\
    } while (false)


/**
 * Encodes a string to UTF-8 and notifies in the case of illegal characters.
 *
 * \param lr     The Listener -- must not be \c NULL.
 * \param mbs    The target string -- must not be \c NULL.
 * \param src    The wide-character source string -- must not be \c NULL.
 * \param len    The maximum number of characters to be written.
 * \param desc   A description of the string -- must not be \c NULL.
 */
#define to_utf8_check(lr, mbs, src, len, desc) do\
    {\
        assert((lr) != NULL);\
        assert((mbs) != NULL);\
        assert((src) != NULL);\
        assert((desc) != NULL);\
        if (to_utf8((mbs), (src), (len)) == EILSEQ)\
        {\
            lo_message msgu_ = lo_message_new();\
            if (msgu_ == NULL)\
            {\
                msg_alloc_fail();\
            }\
            char err_stru_[128] = { '\0' };\
            snprintf(err_stru_, 128, "Illegal characters in %s", (desc));\
            lo_message_add_string(msgu_, err_stru_);\
            int ret = 0;\
            send_msg(lr, "error", msgu_, ret);\
            lo_message_free(msgu_);\
        }\
    } while (false)


/**
 * Decodes a string from UTF-8 and notifies in the case of illegal characters.
 *
 * \param lr     The Listener -- must not be \c NULL.
 * \param wcs    The target string -- must not be \c NULL.
 * \param src    The UTF-8 source string -- must not be \c NULL.
 * \param len    The maximum number of characters to be written.
 * \param desc   A description of the string -- must not be \c NULL.
 */
#define from_utf8_check(lr, wcs, src, len, desc) do\
    {\
        assert((lr) != NULL);\
        assert((wcs) != NULL);\
        assert((src) != NULL);\
        assert((desc) != NULL);\
        if (from_utf8((wcs), (src), (len)) == EILSEQ)\
        {\
            lo_message msgu_ = lo_message_new();\
            if (msgu_ == NULL)\
            {\
                msg_alloc_fail();\
            }\
            char err_stru_[128] = { '\0' };\
            snprintf(err_stru_, 128, "Illegal character sequence in %s",\
                    (desc));\
            lo_message_add_string(msgu_, err_stru_);\
            int ret = 0;\
            send_msg(lr, "error", msgu_, ret);\
            lo_message_free(msgu_);\
        }\
    } while(false)


/**
 * Registers a host application that uses Kunquat.
 *
 * The following OSC parameters are expected:
 * 
 * \li \c s   The OSC URL of the host application with base path.
 *
 * The Listener sends a confirmation message to the host on success.
 */
Listener_callback Listener_register_host;


/**
 * Gets the Kunquat version.
 */
Listener_callback Listener_version;


/**
 * Quits Kunquat.
 */
Listener_callback Listener_quit;


/**
 * Shows all the OSC methods of Kunquat.
 */
Listener_callback Listener_help;


/**
 * A fallback method. A host, if registered, will be sent a notification.
 */
Listener_callback Listener_fallback;


/**
 * Set the number of Voices.
 *
 * The following OSC parameters are expected:
 *
 * \li \c i   The number of Voices. This should be > \c 0 and
 *            <= \c MAX_VOICES.
 */
Listener_callback Listener_set_voices;


#endif // K_LISTENER_H


