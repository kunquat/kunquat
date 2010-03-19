

/*
 * Authors: Heikki Aitakangas, Finland 2009
 *          Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef AUDIO_OPENAL_H
#define AUDIO_OPENAL_H


#include <stdbool.h>
#include <stdint.h>

#include <Audio.h>


typedef struct Audio_openal Audio_openal;


/**
 * Creates a new OpenAL client.
 *
 * \return   The new OpenAL client if successful, otherwise \c NULL.
 */
Audio* new_Audio_openal(void);


#endif // AUDIO_OPENAL_H


