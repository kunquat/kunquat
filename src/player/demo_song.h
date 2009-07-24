

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


#ifndef K_DEMO_SONG_H
#define K_DEMO_SONG_H


#include <stdint.h>

#include <kunquat.h>


/**
 * Inserts the demo song into a Playlist.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
Song* demo_song_create(void);


#endif // K_DEMO_SONG_H


