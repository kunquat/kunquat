

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

    params->do_jump = false;
    params->jump_counter = 0;
    params->jump_target_piref.pat = -1;
    params->jump_target_piref.inst = -1;
    Tstamp_init(&params->jump_target_row);

    params->active_voices = 0;

    return;
}


Master_params* Master_params_preinit(Master_params* params)
{
    assert(params != NULL);

    General_state_preinit(&params->parent);
    params->module = NULL;

    return params;
}


Master_params* Master_params_init(Master_params* params, const Module* module)
{
    assert(params != NULL);
    assert(module != NULL);

    // Sanitise fields
    params->playback_id = 1;

    Master_params_clear(params);

    params->module = module;

    // Init fields
    if (General_state_init(&params->parent, true, params->module->env) == NULL)
    {
        General_state_deinit(&params->parent);
        return NULL;
    }

    return params;
}


void Master_params_reset(Master_params* params)
{
    assert(params != NULL);

    ++params->playback_id;

    Master_params_clear(params);

    General_state_reset(&params->parent);

    params->start_pos.track = 0; // TODO: init start_pos from argument
    params->cur_pos = params->start_pos;

    // Get starting tempo
    const Track_list* tl = Module_get_track_list(params->module);
    if (tl != NULL && params->cur_pos.track < (int16_t)Track_list_get_len(tl))
    {
        const int16_t cur_song = Track_list_get_song_index(
                tl,
                params->cur_pos.track);
        Subsong_table* ss_table = Module_get_subsongs(params->module);
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

    General_state_deinit(&params->parent);

    return;
}


