

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef AUDIO_NULL_H
#define AUDIO_NULL_H


#include <Audio.h>


typedef struct Audio_null Audio_null;


/**
 * Creates a new null audio client.
 *
 * \return   The new null audio client if successful, otherwise \c NULL.
 */
Audio* new_Audio_null(void);


#endif // AUDIO_NULL_H


