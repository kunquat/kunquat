

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Master_params.h>

#include <debug/assert.h>
#include <init/Module.h>
#include <init/sheet/Track_list.h>
#include <mathnum/Random.h>
#include <player/Tuning_state.h>

#include <stdbool.h>
#include <stdlib.h>


#define KQT_JUMP_CONTEXTS_MAX 64


static void Master_params_clear(Master_params* params)
{
    rassert(params != NULL);

    Position_init(&params->start_pos);

    params->playback_state = PLAYBACK_SONG;
    params->is_infinite = false;
    params->pattern_playback_flag = false;

    Random_set_seed(&params->random, params->parent.module->random_seed);

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

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        params->dc_block_state[port].feedforward = 0;
        params->dc_block_state[port].feedback = 0;
    }

    params->volume = 1.0;
    Slider_init(&params->volume_slider, SLIDE_MODE_EXP);

    params->do_jump = false;
    params->jump_counter = 0;
    params->jump_target_piref.pat = -1;
    params->jump_target_piref.inst = -1;
    Tstamp_init(&params->jump_target_row);
    Active_jumps_reset(params->active_jumps, params->jump_cache);

    params->do_goto = false;
    params->goto_safety_counter = 0;
    params->goto_target_piref.pat = -1;
    params->goto_target_piref.inst = -1;
    Tstamp_init(&params->goto_target_row);

    params->cur_tuning_state = -1;
    for (int i = 0; i < KQT_TUNING_TABLES_MAX; ++i)
    {
        if (params->tuning_states[i] != NULL)
            Tuning_state_reset(params->tuning_states[i], NULL);
    }

    params->active_voices = 0;

    return;
}


Master_params* Master_params_preinit(Master_params* params)
{
    rassert(params != NULL);

    General_state_preinit(&params->parent);

    params->active_jumps = NULL;
    params->jump_cache = NULL;

    Random_init(&params->random, "m");

    for (int i = 0; i < KQT_TUNING_TABLES_MAX; ++i)
        params->tuning_states[i] = NULL;

    return params;
}


Master_params* Master_params_init(
        Master_params* params, const Module* module, Env_state* estate)
{
    rassert(params != NULL);
    rassert(module != NULL);
    rassert(estate != NULL);

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
    if ((params->jump_cache == NULL) || (params->active_jumps == NULL))
    {
        Master_params_deinit(params);
        return NULL;
    }

    Master_params_clear(params);

    return params;
}


void Master_params_set_starting_tempo(Master_params* params)
{
    rassert(params != NULL);

    const Track_list* tl = Module_get_track_list(params->parent.module);
    if (tl != NULL)
    {
        if (0 <= params->cur_pos.track &&
                params->cur_pos.track < (int16_t)Track_list_get_len(tl))
        {
            const int cur_song = Track_list_get_song_index(tl, params->cur_pos.track);
            Song_table* song_table = Module_get_songs(params->parent.module);
            Song* song = Song_table_get(song_table, cur_song);

            if (song != NULL)
                params->tempo = Song_get_tempo(song);
        }
        else if (params->playback_state == PLAYBACK_PATTERN)
        {
            // Find the song that contains selected pattern
            Song_table* song_table = Module_get_songs(params->parent.module);
            for (int i = 0; i < Track_list_get_len(tl); ++i)
            {
                const int song_index = Track_list_get_song_index(tl, i);
                const Order_list* ol =
                    Module_get_order_list(params->parent.module, song_index);

                if (Order_list_contains_pat_inst_ref(ol, &params->cur_pos.piref))
                {
                    Song* song = Song_table_get(song_table, song_index);

                    if (song != NULL)
                        params->tempo = Song_get_tempo(song);

                    break;
                }
            }
        }
    }

    return;
}


void Master_params_reset(Master_params* params)
{
    rassert(params != NULL);

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
    rassert(params != NULL);

    if (params->active_jumps != NULL && params->jump_cache != NULL)
        Active_jumps_reset(params->active_jumps, params->jump_cache);

    for (int i = 0; i < KQT_TUNING_TABLES_MAX; ++i)
    {
        if (params->tuning_states[i] != NULL)
        {
            del_Tuning_state(params->tuning_states[i]);
            params->tuning_states[i] = NULL;
        }
    }

    del_Active_jumps(params->active_jumps);
    del_Jump_cache(params->jump_cache);
    params->active_jumps = NULL;
    params->jump_cache = NULL;

    General_state_deinit(&params->parent);

    return;
}


