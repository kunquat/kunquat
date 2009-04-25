

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#ifndef AUDIO_JACK_H
#define AUDIO_JACK_H


#ifdef ENABLE_JACK


#include <stdbool.h>

#include <jack/jack.h>

#include <kunquat.h>


/**
 * Creates a JACK client.
 *
 * \param pl   The Playlist -- must not be \c NULL.
 *
 * \return   The JACK context if successful, or \c NULL if failed.
 */
bool Audio_jack_open(Playlist* pl);


/**
 * Closes the JACK client.
 *
 * \param context   Context data -- must not be \c NULL.
 */
void Audio_jack_close(void);


#else


#endif // ENABLE_JACK


#endif // AUDIO_JACK_H


