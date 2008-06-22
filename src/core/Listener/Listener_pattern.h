

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


#include "Listener.h"


/**
 * The response method <host_path>/pat_info contains the following
 * arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..PATTERNS_MAX-1).
 * \li \c h   The beat count of the Pattern length.
 * \li \c i   The remainder of the Pattern length.
 */


/**
 * The Pattern part of the Listener will respond to most methods with the
 * response method <host_path>/event_info which contains the following
 * arguments if the host call has succeeded:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..PATTERNS_MAX-1).
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
Listener_callback Listener_get_pattern;


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
Listener_callback Listener_get_pats;


/**
 * Creates a new Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. An existing Pattern at this location will be
 *            removed.
 */
Listener_callback Listener_new_pat;


/**
 * Sets the length of the Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. A new Pattern will be created if necessary.
 * \li \c h   The number of whole beats in the Pattern.
 * \li \c i   The remainder of the length.
 *
 * The response method is <host_path>/pat_meta which contains the same
 * arguments.
 */
Listener_callback Listener_set_pat_len;


/**
 * Inserts an Event into a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number. A new Pattern will be created if necessary.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c i   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */
Listener_callback Listener_pat_ins_event;


/**
 * Modifies an existing Event in a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 * \li \c i   The Event type.
 * \li        Zero or more additional arguments depending on the Event type.
 */
Listener_callback Listener_pat_mod_event;


/**
 * Removes an existing Event from a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the Event.
 * \li \c i   The fine-grain position of the Event (0..RELTIME_FULL_PART-1)
 * \li \c i   The (0-based) order of the Event in this location.
 */
Listener_callback Listener_pat_del_event;


/**
 * Removes a row from a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the row.
 * \li \c i   The fine-grain position of the row (0..RELTIME_FULL_PART-1)
 */
Listener_callback Listener_pat_del_row;


/**
 * Shifts Events backward in time.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the start of the shift. Events that would
 *            move past the shift point are removed.
 * \li \c i   The fine-grain position of the start point
 *            (0..RELTIME_FULL_PART-1)
 * \li \c h   The count of beats to be shifted.
 * \li \c i   The fine-grain position fo the shift length
 *            (0..RELTIME_FULL_PART-1)
 */
Listener_callback Listener_pat_shift_up;


/**
 * Shifts Events forward in time.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number.
 * \li \c i   The Column number -- must be between (0..64). 0 is the global
 *            Event column.
 * \li \c h   The beat number of the start of the shift.
 * \li \c i   The fine-grain position of the start point
 *            (0..RELTIME_FULL_PART-1)
 * \li \c h   The count of beats to be shifted.
 * \li \c i   The fine-grain position fo the shift length
 *            (0..RELTIME_FULL_PART-1)
 */
Listener_callback Listener_pat_shift_down;


/**
 * Removes a Pattern.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Pattern number (0..255).
 */
Listener_callback Listener_del_pat;


#endif // K_LISTENER_PATTERN_H


