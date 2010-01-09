

/*
 * Copyright 2010 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
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


