

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Module.h>
#include <player/Master_params.h>
#include <Track_list.h>
#include <xassert.h>


Master_params* Master_params_init(Master_params* params, Environment* env)
{
    assert(params != NULL);
    assert(env != NULL);

    // Sanitise fields
    params->playback_state = PLAYBACK_SONG;
    Position_init(&params->cur_pos);
    params->tempo = 120;

    params->bind = NULL;
    params->active_voices = 0;

    // Init fields
    if (General_state_init(&params->parent, true, env) == NULL)
    {
        General_state_uninit(&params->parent);
        return NULL;
    }

    return params;
}


void Master_params_reset(Master_params* params, const Module* module)
{
    assert(params != NULL);
    assert(module != NULL);

    General_state_reset(&params->parent);

    Position start_pos;
    Position_init(&start_pos);
    start_pos.track = 0;
    params->cur_pos = start_pos;

    // Get starting tempo
    params->tempo = 120;
    const Track_list* tl = Module_get_track_list(module);
    if (tl != NULL && params->cur_pos.track < (int16_t)Track_list_get_len(tl))
    {
        const int16_t cur_song = Track_list_get_song_index(
                tl,
                params->cur_pos.track);
        Subsong_table* ss_table = Module_get_subsongs(module);
        Subsong* ss = Subsong_table_get(ss_table, cur_song);
        if (ss != NULL)
        {
            params->tempo = Subsong_get_tempo(ss);
        }
    }

    return;
}


void Master_params_deinit(Master_params* params)
{
    assert(params != NULL);

    General_state_uninit(&params->parent);

    return;
}


