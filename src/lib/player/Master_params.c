

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


#include <player/Master_params.h>
#include <xassert.h>


Master_params* Master_params_init(Master_params* params, Environment* env)
{
    assert(params != NULL);
    assert(env != NULL);

    // Sanitise fields
    params->playback_state = PLAYBACK_SONG;

    Position_init(&params->cur_pos);

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


void Master_params_reset(Master_params* params)
{
    assert(params != NULL);

    Position start_pos;
    Position_init(&start_pos);
    start_pos.track = 0;
    params->cur_pos = start_pos;

    General_state_reset(&params->parent);

    return;
}


void Master_params_deinit(Master_params* params)
{
    assert(params != NULL);

    General_state_uninit(&params->parent);

    return;
}


