

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_STATE_VOLUME_H
#define K_VOICE_STATE_VOLUME_H


#include <player/Linear_controls.h>
#include <player/Voice_state.h>


typedef struct Voice_state_volume
{
    Voice_state parent;
    Linear_controls volume;
} Voice_state_volume;


#endif // K_VOICE_STATE_VOLUME_H


