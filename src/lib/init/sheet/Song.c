

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Song.h>

#include <debug/assert.h>
#include <init/sheet/song_defaults.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string/Streader.h>

#include <math.h>
#include <stdlib.h>


struct Song
{
    double tempo;
    double global_vol;
};


Song* new_Song(void)
{
    Song* song = memory_alloc_item(Song);
    if (song == NULL)
        return NULL;

    song->tempo = SONG_DEFAULT_TEMPO;
    song->global_vol = SONG_DEFAULT_GLOBAL_VOL;

    return song;
}


bool Song_read_tempo(Song* song, Streader* sr)
{
    rassert(song != NULL);
    rassert(sr != NULL);

    double tempo = NAN;

    if (!Streader_read_float(sr, &tempo))
        return false;

    if ((tempo < 1) || (tempo > 999))
    {
        Streader_set_error(
                sr, "Tempo %.2f is outside valid range [1, 999]", tempo);
        return false;
    }

    song->tempo = tempo;

    return true;
}


double Song_get_tempo(const Song* song)
{
    rassert(song != NULL);
    return song->tempo;
}


bool Song_read_global_vol(Song* song, Streader* sr)
{
    rassert(song != NULL);
    rassert(sr != NULL);

    double vol = NAN;

    if (!Streader_read_float(sr, &vol))
        return false;

    // TODO: decide valid range

    song->global_vol = vol;

    return true;
}


double Song_get_global_vol(const Song* song)
{
    rassert(song != NULL);
    return song->global_vol;
}


void del_Song(Song* song)
{
    if (song == NULL)
        return;

    memory_free(song);

    return;
}


