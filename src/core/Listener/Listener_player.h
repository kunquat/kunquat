

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


#ifndef K_LISTENER_PLAYER_H
#define K_LISTENER_PLAYER_H


#include "Listener.h"


/**
 * All player methods respond with one or more calls of the method
 * <host_path>/player_state which contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c s   Textual description of the playback state. The values are:
 *            "stop", "song", "pattern", "event" and "sample".
 */


/**
 * Stops all playback.
 */
int Listener_stop(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Stops playback associated with the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 */
int Listener_stop_song(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Plays the Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 */
int Listener_play_song(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Plays the given subsong of the Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number (0..255).
 */
int Listener_play_subsong(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Plays one Pattern repeatedly.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..1023).
 */
int Listener_play_pattern(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Plays one Event.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Channel number -- must be between (1..64).
 * \li \c i   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */
int Listener_play_event(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_PLAYER_H


