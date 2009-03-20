

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
 * Most player methods respond with one or more calls of the method
 * <host_path>/player_state which contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c s   Textual description of the playback state. The values are:
 *            "stop", "song", "pattern", "event" and "sample".
 */


/**
 * Stops all playback.
 */
Listener_callback Listener_stop;


/**
 * Stops playback associated with the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 */
Listener_callback Listener_stop_song;


/**
 * Plays the Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 */
Listener_callback Listener_play_song;


/**
 * Plays the given subsong of the Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number (0..255).
 */
Listener_callback Listener_play_subsong;


/**
 * Plays one Pattern repeatedly.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..1023).
 */
Listener_callback Listener_play_pattern;


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
Listener_callback Listener_play_event;


/**
 * Gets statistics from the player.
 *
 * No OSC arguments are expected from the caller.
 *
 * The response method is <host_path>/play_stats and contains the following
 * arguments:
 *
 * \li \c i   Number of channels mixed.
 *
 * For each channel:
 *
 * \li \c d   The maximum amplitude value since the last call. Values above
 *            1.0 indicate clipping.
 * \li \c d   The minimum amplitude value since the last call. Values below
 *            -1.0 indicate clipping.
 *
 * For each Song:
 * 
 * \li \c i   The Song ID.
 * \li \c T/F True if and only if there is anything playing, in which case:
 * \li \c i   The maximum number of Voices used since the last call.
 * \li \c T/F True if and only if there is a Pattern playing, in which case:
 * \li \c i   The Pattern number.
 * \li \c h   The Pattern playback beat.
 * \li \c i   The Pattern playback position remainder.
 * \li \c T/F True if and only if there is a subsong playing, in which case:
 * \li \c i   The subsong number.
 * \li \c i   The order index.
 */
Listener_callback Listener_play_stats;


#endif // K_LISTENER_PLAYER_H


