

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_VOICE_PARAMS_H
#define K_VOICE_PARAMS_H


#include <stdbool.h>

#include <Reltime.h>


typedef struct Voice_params
{
    bool channel_mute;
} Voice_params;


/**
 * Initialises Voice parameters.
 *
 * \param params   The Voice parameters -- must not be \c NULL.
 *
 * \return   The parameter \a params.
 */
Voice_params* Voice_params_init(Voice_params* params);


/**
 * Copies Voice parameters.
 *
 * \param dest   The destination parameters -- must not be \c NULL.
 * \param src    The source parameters -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Voice_params* Voice_params_copy(Voice_params* dest, Voice_params* src);


#endif // K_VOICE_PARAMS_H


