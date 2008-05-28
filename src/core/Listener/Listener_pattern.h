

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


#ifndef K_LISTENER_PATTERN_H
#define K_LISTENER_PATTERN_H


/**
 * The Pattern part of the Listener will respond to most methods with the
 * response method <host_path>/event_info which contains the following
 * arguments if the host call has succeeded:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..255).
 * \li \c i   The Column number (0..64). 0 is the global Event column.
 * \li \c h   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c s   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */


/**
 * Gets information on one Pattern of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 *
 * The response consists of one call of <host_path>/pat_info, zero or more
 * <host_path>/event_info calls and one <host_path>/events_sent call.
 */
int Listener_get_pattern(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Gets information on all the Patterns of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 *
 * The response consists of one pat_info method call for each existing
 * Pattern.
 */
int Listener_get_pats(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Creates a new Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. An existing Pattern at this location will be
 *            removed.
 */
int Listener_new_pat(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Sets an Event in a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. A new Pattern will be created if necessary.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c i   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c s   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */
int Listener_pat_set_event(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Removes a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..255).
 */
int Listener_del_pat(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_PATTERN_H


