

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
#include <string.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_pattern.h"

#include <Song.h>
#include <Pattern.h>
#include <Reltime.h>
#include <Event.h>


/**
 * Sends the event_info message.
 *
 * \param lr         The Listener -- must not be \c NULL.
 * \param song_id    The Song ID.
 * \param pat_num    The Pattern number.
 * \param ch_num     The Channel number.
 * \param index      The 0-based order of the Event in this location.
 * \param event      The Event -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool event_info(Listener* lr,
        int32_t song_id,
        int32_t pat_num,
        int32_t ch_num,
        int32_t index,
        Event* event);


/**
 * Sends the pat_info message (also calls event_info if needed).
 *
 * \param lr        The Listener -- must not be \c NULL.
 * \param song_id   The Song ID.
 * \param pat_num   The Pattern number.
 * \param pat       The Pattern.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool pat_info(Listener* lr,
        int32_t song_id,
        int32_t pat_num,
        Pattern* pat);


/**
 * Checks that the given Event reference is valid.
 *
 * \param lr           The Listener -- must not be \c NULL.
 * \param types        The OSC type description -- must not be \c NULL.
 * \param argv         The OSC arguments -- must not be \c NULL.
 * \param song         Location for the Song to be set -- must not be \c NULL.
 * \param pat          Location for the Pattern to be set -- must not be
 *                     \c NULL.
 * \param expect_pat   \c true iff the Pattern must already exist. If this is
 *                     \c false, a new Pattern will be created if needed.
 * \param col          Location for the Column to be set -- must not be
 *                     \c NULL.
 * \param pos          The Event position to be set -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
static bool check_event_reference(Listener* lr,
        const char* types,
        lo_arg** argv,
        Song** song,
        Pattern** pat,
        bool expect_pat,
        Column** col,
        Reltime* pos);


/**
 * Checks that the given Event data is valid.
 *
 * \param lr       The Listener -- must not be \c NULL.
 * \param types    The OSC type description -- must not be \c NULL.
 * \param argv     The OSC arguments -- must not be \c NULL.
 * \param argc     Number of OSC arguments.
 * \param global   \c true iff the Event is located in the global Event
 *                 channel.
 * \param event    The Event to be set -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a event will not be
 *           altered if the function fails.
 */
static bool check_event_data(Listener* lr,
        const char* types,
        lo_arg** argv,
        int argc,
        bool global,
        Event* event);


int Listener_get_pattern(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    Song* song = Player_get_song(lr->player_cur);
    Pat_table* table = Song_get_pats(song);
    Pattern* pat = Pat_table_get(table, pat_num);
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_set_pat_len(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    Song* song = Player_get_song(lr->player_cur);
    Pat_table* table = Song_get_pats(song);
    Pattern* pat = Pat_table_get(table, pat_num);
    if (pat == NULL)
    {
        pat = new_Pattern();
        if (pat == NULL)
        {
            send_memory_fail(lr, "the new Pattern");
        }
        if (!Pat_table_set(table, pat_num, pat))
        {
            del_Pattern(pat);
            send_memory_fail(lr, "the new Pattern");
        }
        assert(pat != NULL);
    }
    int64_t beats = argv[2]->h;
    int32_t rem = argv[3]->i;
    check_cond(lr, rem >= 0 && rem < RELTIME_BEAT,
            "The Pattern length remainder (%ld)", (long)rem);
    Reltime* len = Reltime_set(RELTIME_AUTO, beats, rem);
    Pattern_set_length(pat, len);
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    lo_message_add_int64(m, beats);
    lo_message_add_int32(m, rem);
    int ret = 0;
    send_msg(lr, "pat_meta", m, ret);
    lo_message_free(m);
    return 0;
}


static bool check_event_reference(Listener* lr,
        const char* types,
        lo_arg** argv,
        Song** song,
        Pattern** pat,
        bool expect_pat,
        Column** col,
        Reltime* pos)
{
    assert(lr != NULL);
    assert(types != NULL);
    assert(argv != NULL);
    assert(song != NULL);
    assert(pat != NULL);
    assert(col != NULL);
    assert(pos != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    *song = Player_get_song(lr->player_cur);
    Pat_table* table = Song_get_pats(*song);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    *pat = Pat_table_get(table, pat_num);
    if (*pat == NULL)
    {
        if (expect_pat)
        {
            lo_message m = new_msg();
            lo_message_add_string(m, "Couldn't find the Pattern");
            int ret = 0;
            send_msg(lr, "error", m, ret);
            lo_message_free(m);
            return false;
        }
        *pat = new_Pattern();
        if (*pat == NULL)
        {
            send_memory_fail(lr, "the new Pattern");
        }
        if (!Pat_table_set(table, pat_num, *pat))
        {
            del_Pattern(*pat);
            send_memory_fail(lr, "the new Pattern");
        }
    }
    int32_t col_num = argv[2]->i;
    check_cond(lr, col_num >= 0 && col_num <= COLUMNS_MAX,
            "The Column number (%ld)", (long)col_num);
    *col = Pattern_global(*pat);
    if (col_num > 0)
    {
        *col = Pattern_col(*pat, col_num - 1);
    }
    int64_t beats = argv[3]->h;
    int32_t rem = argv[4]->i;
    check_cond(lr, rem >= 0 && rem < RELTIME_BEAT,
            "The row position remainder (%ld)", (long)rem);
    Reltime_set(pos, beats, rem);
    int32_t event_order = argv[5]->i;
    check_cond(lr, event_order >= 0,
            "The Event order index (%ld)", (long)event_order);
    return true;
}


static bool check_event_data(Listener* lr,
        const char* types,
        lo_arg** argv,
        int argc,
        bool global,
        Event* event)
{
    assert(lr != NULL);
    assert(types != NULL);
    assert(argv != NULL);
    int32_t event_type = argv[6]->i;
    check_cond(lr, EVENT_TYPE_IS_VALID(event_type),
            "The Event type (%ld)", (long)event_type);
    check_cond(lr, EVENT_TYPE_IS_GENERAL(event_type)
            || (global == EVENT_TYPE_IS_GLOBAL(event_type)),
            "The Event type (%ld) is unsupported in this channel -- it",
            (long)event_type);
    char* type_desc = Event_type_get_field_types(event_type);
    check_cond(lr, type_desc != NULL,
            "The Event type (%ld) is unused -- the type description",
            (long)event_type);
    int field_count = strlen(type_desc);
    int num_args = argc - 7;
    check_cond(lr, num_args == field_count,
            "The number of Event parameters (%d)", num_args);
    Event_reset(event, event_type);
    for (int i = 0; i < field_count; ++i)
    {
        char type = types[7 + i];
        if (type_desc[i] == 'i')
        {
            check_cond(lr, type == 'h',
                    "The type of the Event field #%d (%c)", i, type);
            if (!Event_set_int(event, i, argv[7 + i]->h))
            {
                lo_message m = new_msg();
                lo_message_add_string(m, "Invalid value of the field");
                lo_message_add_int32(m, i);
                lo_message_add_int32(m, argv[7 + i]->h);
                int ret = 0;
                send_msg(lr, "error", m, ret);
                lo_message_free(m);
                return false;
            }
        }
        else if (type_desc[i] == 'f')
        {
            check_cond(lr, type == 'd',
                    "The type of the Event field #%d (%c)", i, type);
            if (!Event_set_float(event, i, argv[7 + i]->d))
            {
                lo_message m = new_msg();
                lo_message_add_string(m, "Invalid value of the field");
                lo_message_add_int32(m, i);
                lo_message_add_double(m, argv[7 + i]->d);
                int ret = 0;
                send_msg(lr, "error", m, ret);
                lo_message_free(m);
                return false;
            }
        }
    }
    return true;
}


int Listener_pat_del_event(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    Song* song = NULL;
    Pattern* pat = NULL;
    Column* col = NULL;
    Reltime* pos = Reltime_init(RELTIME_AUTO);
    if (!check_event_reference(lr, types, argv, &song, &pat, true, &col, pos))
    {
        return 0;
    }
    assert(song != NULL);
    assert(pat != NULL);
    assert(col != NULL);
    int32_t song_id = argv[0]->i;
    int32_t pat_num = argv[1]->i;
    int32_t event_order = argv[5]->i;
    Event* event = Column_get_edit(col, pos);
    check_cond(lr, event != NULL && Reltime_cmp(pos, Event_pos(event)) == 0,
            "No Event found -- the Event retrieved (%p)", (void*)event);
    for (int32_t i = 0; i < event_order; ++i)
    {
        event = Column_get_next_edit(col);
        check_cond(lr, event != NULL && Reltime_cmp(pos, Event_pos(event)) == 0,
                "No Event found -- the Event retrieved (%p)", (void*)event);
    }
    bool column_removed = Column_remove(col, event);
    check_cond(lr, column_removed,
            "No Event in the given location -- the result (%s)", "false");
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_pat_mod_event(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    check_cond(lr, strncmp(types, "iiihiii", 7) == 0,
            "The argument type list (%s)", types);
    Song* song = NULL;
    Pattern* pat = NULL;
    Column* col = NULL;
    Reltime* pos = Reltime_init(RELTIME_AUTO);
    if (!check_event_reference(lr, types, argv, &song, &pat, true, &col, pos))
    {
        return 0;
    }
    assert(song != NULL);
    assert(pat != NULL);
    assert(col != NULL);
    int32_t song_id = argv[0]->i;
    int32_t pat_num = argv[1]->i;
    int32_t col_num = argv[2]->i;
    int32_t event_order = argv[5]->i;
    int32_t event_type = argv[6]->i;
    check_cond(lr, EVENT_TYPE_IS_VALID(event_type),
            "The Event type (%ld)", (long)event_type);
    Event* event = Column_get_edit(col, pos);
    check_cond(lr, event != NULL && Reltime_cmp(pos, Event_pos(event)) == 0,
            "No Event found -- the Event retrieved (%p)", (void*)event);
    for (int32_t i = 0; i < event_order; ++i)
    {
        event = Column_get_next_edit(col);
        check_cond(lr, event != NULL && Reltime_cmp(pos, Event_pos(event)) == 0,
                "No Event found -- the Event retrieved (%p)", (void*)event);
    }
    union { int64_t i; double d; } orig_values[EVENT_FIELDS];
    Event_type orig_type = Event_get_type(event);
    char* orig_types = Event_type_get_field_types(orig_type);
    int orig_count = 0;
    if (orig_types != NULL)
    {
        orig_count = strlen(orig_types);
    }
    for (int i = 0; i < orig_count; ++i)
    {
        bool got_val = false;
        if (orig_types[i] == 'i')
        {
            got_val = Event_int(event, i, &orig_values[i].i);
        }
        else if (orig_types[i] == 'f')
        {
            got_val = Event_float(event, i, &orig_values[i].d);
        }
        assert(got_val);
    }
    if (!check_event_data(lr, types, argv, argc, col_num == 0, event))
    {
        Event_reset(event, orig_type);
        for (int i = 0; i < orig_count; ++i)
        {
            bool set_val = false;
            if (orig_types[i] == 'i')
            {
                set_val = Event_set_int(event, i, orig_values[i].i);
            }
            else if (orig_types[i] == 'f')
            {
                set_val = Event_set_float(event, i, orig_values[i].d);
            }
            assert(set_val);
        }
        return 0;
    }
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_pat_ins_event(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    check_cond(lr, strncmp(types, "iiihiii", 7) == 0,
            "The argument type list (%s)", types);
    Song* song = NULL;
    Pattern* pat = NULL;
    Column* col = NULL;
    Reltime* pos = Reltime_init(RELTIME_AUTO);
    if (!check_event_reference(lr, types, argv, &song, &pat, false, &col, pos))
    {
        return 0;
    }
    assert(song != NULL);
    assert(pat != NULL);
    assert(col != NULL);
    int32_t song_id = argv[0]->i;
    int32_t pat_num = argv[1]->i;
    int32_t col_num = argv[2]->i;
    int32_t event_order = argv[5]->i;
    int32_t event_type = argv[6]->i;
    check_cond(lr, EVENT_TYPE_IS_VALID(event_type),
            "The Event type (%ld)", (long)event_type);
    Event* event = new_Event(pos, event_type);
    if (event == NULL)
    {
        send_memory_fail(lr, "the new Event");
    }
    if (!check_event_data(lr, types, argv, argc, col_num == 0, event))
    {
        del_Event(event);
        return 0;
    }
    if (!Column_ins(col, event))
    {
        del_Event(event);
        send_memory_fail(lr, "the new Event");
    }
    Column_move(col, event, event_order);
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_pat_del_row(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    Song* song = Player_get_song(lr->player_cur);
    Pat_table* table = Song_get_pats(song);
    Pattern* pat = Pat_table_get(table, pat_num);
    int32_t col_num = argv[2]->i;
    check_cond(lr, col_num >= 0 && col_num <= COLUMNS_MAX,
            "The Column number (%ld)", (long)col_num);
    Column* col = Pattern_global(pat);
    if (col_num > 0)
    {
        col = Pattern_col(pat, col_num - 1);
    }
    int64_t beats = argv[3]->h;
    int32_t rem = argv[4]->i;
    check_cond(lr, rem >= 0 && rem < RELTIME_BEAT,
            "The row position remainder (%ld)", (long)rem);
    Reltime* pos = Reltime_set(RELTIME_AUTO, beats, rem);
    if (Column_remove_row(col, pos))
    {
        if (!pat_info(lr, song_id, pat_num, pat))
        {
            return 0;
        }
        lo_message m = new_msg();
        lo_message_add_int32(m, song_id);
        lo_message_add_int32(m, pat_num);
        int ret = 0;
        send_msg(lr, "events_sent", m, ret);
        lo_message_free(m);
    }
    return 0;
}


int Listener_pat_shift_up(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    Pat_table* table = Song_get_pats(song);
    Pattern* pat = Pat_table_get(table, pat_num);
    int32_t col_num = argv[2]->i;
    check_cond(lr, col_num >= 0 && col_num <= COLUMNS_MAX,
            "The Column number (%ld)", (long)col_num);
    Column* col = Pattern_global(pat);
    if (col_num > 0)
    {
        col = Pattern_col(pat, col_num - 1);
    }
    int64_t beats = argv[3]->h;
    int32_t rem = argv[4]->i;
    check_cond(lr, rem >= 0 && rem < RELTIME_BEAT,
            "The shift position remainder (%ld)", (long)rem);
    Reltime* pos = Reltime_set(RELTIME_AUTO, beats, rem);
    int64_t len_beats = argv[5]->h;
    int32_t len_rem = argv[6]->i;
    check_cond(lr, len_rem >= 0 && len_rem < RELTIME_BEAT,
            "The shift length remainder (%ld)", (long)len_rem);
    Reltime* len = Reltime_set(RELTIME_AUTO, len_beats, len_rem);
    Column_shift_up(col, pos, len);
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


int Listener_pat_shift_down(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    get_player(lr, song_id, types[0]);
    Song* song = Player_get_song(lr->player_cur);
    int32_t pat_num = argv[1]->i;
    check_cond(lr, pat_num >= 0 && pat_num < PATTERNS_MAX,
            "The Pattern number (%ld)", (long)pat_num);
    Pat_table* table = Song_get_pats(song);
    Pattern* pat = Pat_table_get(table, pat_num);
    int32_t col_num = argv[2]->i;
    check_cond(lr, col_num >= 0 && col_num <= COLUMNS_MAX,
            "The Column number (%ld)", (long)col_num);
    Column* col = Pattern_global(pat);
    if (col_num > 0)
    {
        col = Pattern_col(pat, col_num - 1);
    }
    int64_t beats = argv[3]->h;
    int32_t rem = argv[4]->i;
    check_cond(lr, rem >= 0 && rem < RELTIME_BEAT,
            "The shift position remainder (%ld)", (long)rem);
    Reltime* pos = Reltime_set(RELTIME_AUTO, beats, rem);
    int64_t len_beats = argv[5]->h;
    int32_t len_rem = argv[6]->i;
    check_cond(lr, len_rem >= 0 && len_rem < RELTIME_BEAT,
            "The shift length remainder (%ld)", (long)len_rem);
    Reltime* len = Reltime_set(RELTIME_AUTO, len_beats, len_rem);
    Column_shift_down(col, pos, len);
    if (!pat_info(lr, song_id, pat_num, pat))
    {
        return 0;
    }
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    int ret = 0;
    send_msg(lr, "events_sent", m, ret);
    lo_message_free(m);
    return 0;
}


static bool event_info(Listener* lr,
        int32_t song_id,
        int32_t pat_num,
        int32_t ch_num,
        int32_t index,
        Event* event)
{
    assert(lr != NULL);
    assert(pat_num >= 0);
    assert(ch_num >= 0);
    assert(ch_num <= 64);
    assert(index >= 0);
    assert(event != NULL);
    Reltime* pos = Event_pos(event);
    lo_message m = new_msg();
    lo_message_add_int32(m, song_id);
    lo_message_add_int32(m, pat_num);
    lo_message_add_int32(m, ch_num);
    lo_message_add_int64(m, Reltime_get_beats(pos));
    lo_message_add_int32(m, Reltime_get_rem(pos));
    lo_message_add_int32(m, index);
    lo_message_add_int32(m, event->type);
    bool got_val = false;
    int64_t val = 0;
    switch (event->type)
    {
        case EVENT_TYPE_NOTE_ON:
            for (int i = 0; i < 4; ++i)
            {
                got_val = Event_int(event, i, &val);
                assert(got_val);
                lo_message_add_int64(m, val);
            }
            break;
        case EVENT_TYPE_NOTE_OFF:
            break;
        default:
            break;
    }
    int ret = 0;
    send_msg(lr, "event_info", m, ret);
    lo_message_free(m);
    if (ret == -1)
    {
        return false;
    }
    return true;
}


static bool pat_info(Listener* lr,
        int32_t song_id,
        int32_t pat_num,
        Pattern* pat)
{
    if (pat != NULL)
    {
        Reltime* pat_len = Pattern_get_length(pat);
        lo_message m = new_msg();
        lo_message_add_int32(m, song_id);
        lo_message_add_int32(m, pat_num);
        lo_message_add_int64(m, Reltime_get_beats(pat_len));
        lo_message_add_int32(m, Reltime_get_rem(pat_len));
        int ret = 0;
        send_msg(lr, "pat_info", m, ret);
        lo_message_free(m);
        if (ret == -1)
        {
            return false;
        }
        for (int i = -1; i < COLUMNS_MAX; ++i)
        {
            Column* col = Pattern_global(pat);
            if (i > -1)
            {
                col = Pattern_col(pat, i);
            }
            Event* event = Column_get_edit(col, Reltime_init(RELTIME_AUTO));
            int index = 0;
            Reltime* prev_time = Reltime_set(RELTIME_AUTO, INT64_MIN, 0);
            Reltime* time = RELTIME_AUTO;
            while (event != NULL)
            {
                Reltime_copy(time, Event_pos(event));
                if (Reltime_cmp(prev_time, time) == 0)
                {
                    ++index;
                }
                else
                {
                    index = 0;
                }
                Reltime_copy(prev_time, time);
                if (!event_info(lr, song_id, pat_num, i + 1, index, event))
                {
                    return false;
                }
                event = Column_get_next_edit(col);
            }
        }
    }
    else
    {
        lo_message m = new_msg();
        lo_message_add_int32(m, song_id);
        lo_message_add_int32(m, pat_num);
        lo_message_add_int64(m, 16);
        lo_message_add_int32(m, 0);
        int ret = 0;
        send_msg(lr, "pat_info", m, ret);
        lo_message_free(m);
        if (ret == -1)
        {
            return false;
        }
    }
    return true;
}


