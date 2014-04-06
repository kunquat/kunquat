

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <debug/assert.h>
#include <module/Module.h>
#include <module/sheet/Track_list.h>
#include <player/Master_params.h>


#define KQT_JUMP_CONTEXTS_MAX 64


static void Master_params_clear(Master_params* params)
{
    assert(params != NULL);

    Position_init(&params->start_pos);

    params->playback_state = PLAYBACK_SONG;
    params->is_infinite = false;

    Position_init(&params->cur_pos);
    params->cur_ch = 0;
    params->cur_trigger = 0;

    Tstamp_init(&params->delay_left);

    params->tempo_settings_changed = false;
    params->tempo = 120;
    params->tempo_slide = 0;
    Tstamp_init(&params->tempo_slide_length);
    params->tempo_slide_target = 0;
    Tstamp_init(&params->tempo_slide_left);
    Tstamp_init(&params->tempo_slide_slice_left);
    params->tempo_slide_update = 0;

    params->volume = 1.0;
    Slider_init(&params->volume_slider, SLIDE_MODE_EXP);

    params->do_jump = false;
    params->jump_counter = 0;
    params->jump_target_piref.pat = -1;
    params->jump_target_piref.inst = -1;
    Tstamp_init(&params->jump_target_row);
    Active_jumps_reset(params->active_jumps, params->jump_cache);

    params->active_voices = 0;

    return;
}


Master_params* Master_params_preinit(Master_params* params)
{
    assert(params != NULL);

    General_state_preinit(&params->parent);

    params->active_jumps = NULL;
    params->jump_cache = NULL;

    return params;
}


Master_params* Master_params_init(
        Master_params* params,
        const Module* module,
        Env_state* estate)
{
    assert(params != NULL);
    assert(module != NULL);
    assert(estate != NULL);

    // Sanitise fields
    params->playback_id = 1;

    // Init fields
    if (General_state_init(&params->parent, true, estate, module) == NULL)
    {
        Master_params_deinit(params);
        return NULL;
    }

    params->jump_cache = new_Jump_cache(KQT_JUMP_CONTEXTS_MAX);
    params->active_jumps = new_Active_jumps();
    if (params->jump_cache == NULL || params->active_jumps == NULL)
    {
        Master_params_deinit(params);
        return NULL;
    }

    Master_params_clear(params);

    return params;
}


void Master_params_set_starting_tempo(Master_params* params)
{
    assert(params != NULL);

    const Track_list* tl = Module_get_track_list(params->parent.module);
    if (tl != NULL && params->cur_pos.track < (int16_t)Track_list_get_len(tl))
    {
        const int16_t cur_song = Track_list_get_song_index(
                tl,
                params->cur_pos.track);
        Song_table* song_table = Module_get_songs(params->parent.module);
        Song* song = Song_table_get(song_table, cur_song);

        if (song != NULL)
            params->tempo = Song_get_tempo(song);
    }

    return;
}


void Master_params_reset(Master_params* params)
{
    assert(params != NULL);

    ++params->playback_id;

    Master_params_clear(params);

    General_state_reset(&params->parent);

    params->start_pos.track = 0; // TODO: init start_pos from argument
    params->cur_pos = params->start_pos;

    Master_params_set_starting_tempo(params);

    return;
}


void Master_params_deinit(Master_params* params)
{
    assert(params != NULL);

    if (params->active_jumps != NULL && params->jump_cache != NULL)
        Active_jumps_reset(params->active_jumps, params->jump_cache);

    del_Active_jumps(params->active_jumps);
    del_Jump_cache(params->jump_cache);
    params->active_jumps = NULL;
    params->jump_cache = NULL;

    General_state_deinit(&params->parent);

    return;
}


