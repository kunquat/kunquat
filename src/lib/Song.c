

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent posongible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <File_base.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string_common.h>
#include <Song.h>
#include <xassert.h>


static bool Song_parse(Song* song, char* str, Read_state* state);


Song* new_Song(void)
{
    Song* song = memory_alloc_item(Song);
    if (song == NULL)
        return NULL;

    song->res = 8;
    song->pats = memory_alloc_items(int16_t, song->res);
    if (song->pats == NULL)
    {
        memory_free(song);
        return NULL;
    }

    for (int i = 0; i < song->res; ++i)
        song->pats[i] = KQT_SECTION_NONE;

    song->tempo = SONG_DEFAULT_TEMPO;
    song->global_vol = SONG_DEFAULT_GLOBAL_VOL;
    song->scale = SONG_DEFAULT_SCALE;

    return song;
}


Song* new_Song_from_string(char* str, Read_state* state)
{
    assert(state != NULL);

    Song* song = new_Song();
    if (song == NULL)
        return NULL;

    if (!Song_parse(song, str, state))
    {
        del_Song(song);
        return NULL;
    }

    return song;
}


static bool Song_parse(Song* song, char* str, Read_state* state)
{
    assert(song != NULL);
    assert(state != NULL);

    if (str == NULL)
        return true;

    str = read_const_char(str, '{', state);
    if (state->error)
        return false;

    str = read_const_char(str, '}', state);
    if (!state->error)
        return true;

    Read_state_clear_error(state);
    char key[128] = { '\0' };
    bool expect_pair = true;

    while (expect_pair)
    {
        str = read_string(str, key, 128, state);
        str = read_const_char(str, ':', state);
        if (state->error)
            return false;

        if (string_eq(key, "tempo"))
        {
            str = read_double(str, &song->tempo, state);
            if (state->error)
                return false;

            if (song->tempo < 1 || song->tempo > 999)
            {
                Read_state_set_error(
                        state,
                        "Tempo (%f) is outside valid range",
                        song->tempo);
                return false;
            }
        }
        else if (string_eq(key, "global_vol"))
        {
            str = read_double(str, &song->global_vol, state);
        }
        else if (string_eq(key, "scale"))
        {
            int64_t num = 0;
            str = read_int(str, &num, state);
            if (state->error)
                return false;

            if (num < 0 || num >= KQT_SCALES_MAX)
            {
                Read_state_set_error(
                        state,
                        "Scale number (%" PRId64 ") is outside valid range",
                        num);
                return false;
            }
            song->scale = num;
        }
#if 0
        else if (string_eq(key, "patterns"))
        {
            str = read_const_char(str, '[', state);
            if (state->error)
                return false;

            str = read_const_char(str, ']', state);
            if (state->error)
            {
                Read_state_clear_error(state);
                bool expect_num = true;
                int index = 0;
                while (expect_num && index < KQT_SECTIONS_MAX)
                {
                    int64_t num = 0;
                    str = read_int(str, &num, state);
                    if (state->error)
                        return false;

                    if ((num < 0 || num >= KQT_PATTERNS_MAX) && num != KQT_SECTION_NONE)
                    {
                        Read_state_set_error(state,
                                 "Pattern number (%" PRId64 ") is outside valid range", num);
                        return false;
                    }
                    if (!Song_set(song, index, num))
                    {
                        Read_state_set_error(state,
                                 "Couldn't allocate memory for a Song");
                        return false;
                    }
                    ++index;
                    check_next(str, state, expect_num);
                }
                str = read_const_char(str, ']', state);
                if (state->error)
                    return false;
            }
        }
#endif
        else
        {
            Read_state_set_error(state, "Unrecognised key in Song: %s\n", key);
            return false;
        }
        if (state->error)
        {
            return false;
        }
        check_next(str, state, expect_pair);
    }

    str = read_const_char(str, '}', state);
    if (state->error)
        return false;

    return true;
}


int16_t Song_get_length(Song* song)
{
    assert(song != NULL);

    int length = 0;
    for (length = 0; length < song->res; ++length)
    {
        if (song->pats[length] == KQT_SECTION_NONE)
            break;
    }

    return length;
}


void Song_set_tempo(Song* song, double tempo)
{
    assert(song != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    song->tempo = tempo;

    return;
}


double Song_get_tempo(Song* song)
{
    assert(song != NULL);
    return song->tempo;
}


void Song_set_global_vol(Song* song, double vol)
{
    assert(song != NULL);
    assert(isfinite(vol) || vol == -INFINITY);

    song->global_vol = vol;

    return;
}


double Song_get_global_vol(Song* song)
{
    assert(song != NULL);
    return song->global_vol;
}


void Song_set_scale(Song* song, int index)
{
    assert(song != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    song->scale = index;

    return;
}


int Song_get_scale(Song* song)
{
    assert(song != NULL);
    return song->scale;
}


void del_Song(Song* song)
{
    if (song == NULL)
        return;

    memory_free(song->pats);
    memory_free(song);

    return;
}


