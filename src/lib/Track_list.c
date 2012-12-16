

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <inttypes.h>
#include <stdint.h>

#include <Track_list.h>
#include <Vector.h>
#include <xassert.h>
#include <xmemory.h>


struct Track_list
{
    Vector* songs;
};


Track_list* new_Track_list(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
        return NULL;

    // Create the base structure
    Track_list* tl = xalloc(Track_list);
    if (tl == NULL)
        return NULL;
    tl->songs = NULL;

    // Create song index vector
    tl->songs = new_Vector(sizeof(int16_t));
    if (tl->songs == NULL)
    {
        del_Track_list(tl);
        return NULL;
    }

    // List is empty by default
    if (str == NULL)
        return tl;

    // Read the list of song indices
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Track_list(tl);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        // List not empty, process elements
        Read_state_clear_error(state);

        bool used[KQT_SONGS_MAX] = { false };

        bool expect_index = true;
        while (expect_index)
        {
            // Read the song index
            int64_t num = 0;
            str = read_int(str, &num, state);
            if (state->error)
            {
                del_Track_list(tl);
                return NULL;
            }

            // Check index range
            if (num < 0 || num >= KQT_SONGS_MAX)
            {
                Read_state_set_error(state,
                        "Song index outside range [0, %d)", KQT_SONGS_MAX);
                del_Track_list(tl);
                return NULL;
            }

            // Check if the index is already used
            if (used[num])
            {
                Read_state_set_error(state,
                        "Duplicate occurrence of song index %" PRId64, num);
                del_Track_list(tl);
                return NULL;
            }
            used[num] = true;

            // Add song index
            int16_t song_index = num;
            Vector_append(tl->songs, &song_index);

            check_next(str, state, expect_index);
        }
    }

    return tl;
}


size_t Track_list_get_len(const Track_list* tl)
{
    assert(tl != NULL);
    return Vector_size(tl->songs);
}


int16_t Track_list_get_song_index(const Track_list* tl, size_t index)
{
    assert(tl != NULL);
    assert(index < Track_list_get_len(tl));

    int16_t song_index = -1;
    Vector_get(tl->songs, index, &song_index);
    assert(song_index >= 0);
    return song_index;
}


void del_Track_list(Track_list* tl)
{
    if (tl == NULL)
        return;

    del_Vector(tl->songs);
    xfree(tl);
    return;
}


