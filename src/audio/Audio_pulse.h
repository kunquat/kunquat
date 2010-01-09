

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


#ifndef AUDIO_PULSE_H
#define AUDIO_PULSE_H


#include <stdbool.h>
#include <stdint.h>

#include <Audio.h>


typedef struct Audio_pulse Audio_pulse;


/**
 * Creates a new PulseAudio client.
 *
 * \return   The new PulseAudio client if successful, otherwise \c NULL.
 */
Audio* new_Audio_pulse(void);


#endif // AUDIO_PULSE_H


