

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_DRIVER_ALSA_H
#define K_DRIVER_ALSA_H


#include <stdbool.h>
#include <stdint.h>

#include <Playlist.h>


/**
 * Initialises an ALSA driver.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param freq       A location where the mixing frequency is stored -- must
 *                   not be \c NULL.
 *
 * \return   \c true if initialisation succeeded, otherwise \c false.
 */
bool Driver_alsa_init(Playlist* playlist, uint32_t* freq);


/**
 * Unitialises an ALSA driver.
 */
void Driver_alsa_close(void);


#endif // K_DRIVER_ALSA_H


