

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


#ifndef AUDIO_JACK_H
#define AUDIO_JACK_H


#include <stdbool.h>

#include <Audio.h>


typedef struct Audio_jack Audio_jack;


/**
 * Creates a new JACK client.
 *
 * \return   The new JACK client if successful, otherwise \c NULL.
 */
Audio* new_Audio_jack(void);


#endif // AUDIO_JACK_H


