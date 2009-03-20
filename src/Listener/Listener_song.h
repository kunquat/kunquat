

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


#ifndef K_LISTENER_SONG_H
#define K_LISTENER_SONG_H


#include "Listener.h"


/**
 * Many of the Song methods send a response method <host_path>/song_info which
 * contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c s   The Song title in UTF-8 format.
 * \li \c d   The mixing volume.
 * \li \c i   The number of the initial subsong.
 */


/**
 * Creates a new Song.
 *
 * The response method is <host_path>/new_song.
 *
 * If the new Song is created successfully, an ID number of the new Song is
 * sent. This ID serves as a reference to the Song. In case of an error one or
 * more strings describing the error are sent.
 */
Listener_callback Listener_new_song;


/**
 * Gets IDs of all the Songs.
 *
 * The response method is <host_path>/songs.
 *
 * The response message contains all the Song IDs.
 */
Listener_callback Listener_get_songs;


/**
 * Gets metadata of the given Song. Takes the ID number of the Song as an
 * argument.
 *
 * The response method is <host_path>/song_info.
 */
Listener_callback Listener_get_song_info;


/**
 * Sets the title of the given Song. Takes the ID number of the Song and the
 * new title in UTF-8 format as an argument.
 *
 * The response method is <host_path>/song_info.
 */
Listener_callback Listener_set_song_title;


/**
 * Sets the mixing volume of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c d   The mixing volume (< 0) in dBFS.
 */
Listener_callback Listener_set_mix_vol;


/**
 * Sets the tempo of the given subsong.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li \c d   The tempo.
 */
Listener_callback Listener_set_subsong_tempo;


/**
 * Sets the global volume of the given subsong.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li \c d   The global volume.
 */
Listener_callback Listener_set_subsong_global_vol;


/**
 * Destroys a Song. The method takes one argument, the ID number of the Song.
 *
 * The response method is <host_path>/del_song.
 *
 * If a Song is deleted, the ID number of the Song is sent. Otherwise no
 * parameters are sent.
 */
Listener_callback Listener_del_song;


#endif // K_LISTENER_SONG_H


