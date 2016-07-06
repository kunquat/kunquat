

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Track_list.h>

#include <containers/Vector.h>
#include <debug/assert.h>
#include <memory.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Track_list
{
    Vector* songs;
};


typedef struct read_data
{
    Track_list* tl;
    bool used[KQT_SONGS_MAX];
} read_data;

static bool read_song(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);

    if (index >= KQT_TRACKS_MAX)
    {
        Streader_set_error(sr, "Too many entries in track list");
        return false;
    }

    read_data* rd = userdata;

    // Read the song index
    int64_t num = 0;
    if (!Streader_read_int(sr, &num))
        return false;

    // Check index range
    if (num < 0 || num >= KQT_SONGS_MAX)
    {
        Streader_set_error(
                sr, "Song index outside range [0, %d)", KQT_SONGS_MAX);
        return false;
    }

    // Check if the index is already used
    if (rd->used[num])
    {
        Streader_set_error(
                sr, "Duplicate occurrence of song index %" PRId64, num);
        return false;
    }
    rd->used[num] = true;

    // Add song index
    int16_t song_index = (int16_t)num;
    Vector_append(rd->tl->songs, &song_index);

    return true;
}

Track_list* new_Track_list(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    // Create the base structure
    Track_list* tl = memory_alloc_item(Track_list);
    if (tl == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for track list");
        return NULL;
    }

    tl->songs = NULL;

    // Create song index vector
    tl->songs = new_Vector(sizeof(int16_t));
    if (tl->songs == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for track list");
        del_Track_list(tl);
        return NULL;
    }

    // List is empty by default
    if (!Streader_has_data(sr))
        return tl;

    read_data* rd = &(read_data){ .tl = tl, .used = { false } };

    if (!Streader_read_list(sr, read_song, rd))
    {
        del_Track_list(tl);
        return NULL;
    }

    return tl;
}


int Track_list_get_len(const Track_list* tl)
{
    rassert(tl != NULL);
    return (int)Vector_size(tl->songs);
}


int Track_list_get_song_index(const Track_list* tl, int index)
{
    rassert(tl != NULL);
    rassert(index < Track_list_get_len(tl));

    int16_t song_index = -1;
    Vector_get(tl->songs, index, &song_index);

    rassert(song_index >= 0);
    return song_index;
}


int Track_list_get_track_by_song(const Track_list* tl, int song_index)
{
    rassert(tl != NULL);
    rassert(song_index >= 0);

    for (int64_t i = 0; i < Vector_size(tl->songs); ++i)
    {
        const int16_t* cur_index = Vector_get_ref(tl->songs, i);
        if (*cur_index == song_index)
            return (int)i;
    }

    return -1;
}


void del_Track_list(Track_list* tl)
{
    if (tl == NULL)
        return;

    del_Vector(tl->songs);
    memory_free(tl);

    return;
}


