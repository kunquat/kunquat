

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

#include <kunquat/limits.h>
#include <memory.h>
#include <string_common.h>
#include <Song.h>
#include <xassert.h>


static bool Song_parse(Song* song, Streader* sr);


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


Song* new_Song_from_string(Streader* sr)
{
    assert(sr != NULL);

    Song* song = new_Song();
    if (song == NULL)
        return NULL;

    if (!Song_parse(song, sr))
    {
        del_Song(song);
        return NULL;
    }

    return song;
}


static bool read_song_item(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    Song* song = userdata;

    if (string_eq(key, "tempo"))
    {
        if (!Streader_read_float(sr, &song->tempo))
            return false;

        if (song->tempo < 1 || song->tempo > 999)
        {
            Streader_set_error(
                    sr, "Tempo (%f) is outside valid range", song->tempo);
            return false;
        }
    }
    else if (string_eq(key, "global_vol"))
    {
        if (!Streader_read_float(sr, &song->global_vol))
            return false;
    }
    else if (string_eq(key, "scale"))
    {
        int64_t num = 0;
        if (!Streader_read_int(sr, &num))
            return false;

        if (num < 0 || num >= KQT_SCALES_MAX)
        {
            Streader_set_error(
                    sr,
                    "Scale number (%" PRId64 ") is outside valid range",
                    num);
            return false;
        }
        song->scale = num;
    }
    else
    {
        Streader_set_error(sr, "Unrecognised key in Song: %s\n", key);
        return false;
    }

    return true;
}

static bool Song_parse(Song* song, Streader* sr)
{
    assert(song != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Streader_has_data(sr))
        return true;

    return Streader_read_dict(sr, read_song_item, song);
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


