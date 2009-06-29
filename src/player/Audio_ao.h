

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


#ifndef AUDIO_AO_H
#define AUDIO_AO_H


#ifdef ENABLE_AO


#include <stdbool.h>
#include <stdint.h>

#include <ao/ao.h>

#include <kunquat.h>


/**
 * Creates a libao client.
 *
 * \param pl   The Playlist -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Audio_ao_open(Playlist* pl);


/**
 * Closes the libao client.
 */
void Audio_ao_close(void);


#endif // ENABLE_AO


#endif // AUDIO_AO_H


