

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


#ifndef K_LISTENER_NOTE_TABLE_H
#define K_LISTENER_NOTE_TABLE_H


/**
 * The response method <host_path>/note_table_info contains the following
 * arguments:
 *
 * \li \c i   The Song ID.
 * \li \c s   The name of the Note table (in UTF-8 format).
 * \li \c i   The number of notes in the Note table.
 * \li \c i   The number of note modifiers in the Note table.
 * \li \c i   The initial reference note.
 * \li \c i   The current reference note.
 * \li \c d   The reference pitch.
 * \li \c h/d The numerator of the octave ratio.
 * \li \c h   The denominator of the octave ratio if the numerator is an
 *            integer (h).
 */

/**
 * The response method <host_path>/note_info contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The note number.
 * \li \c s   The name of the note (in UTF-8 format).
 * \li \c T/F True if the pitch is set as ratio, False if in cents.
 * \li        Either:
 * \li \li \c h/d The numerator of the pitch ratio (relative to the reference
 *                pitch).
 * \li \li \c h   The denominator of the pitch ratio.
 * \li        Or:
 * \li \li \c d   The pitch difference in cents (relative to the reference
 *                pitch).
 * \li        Either:
 * \li \li \c h/d The numerator of the retuned pitch ratio.
 * \li \li \c h   The denominator of the retuned pitch ratio if the numerator
 *                is an integer (h).
 * \li        Or:
 * \li \li \c d   The current pitch difference in cents (relative to the
 *                reference pitch).
 */

/**
 * The response method <host_path>/note_mod_info contains the following
 * arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The note modifier number.
 * \li \c s   The name of the note modifier (in UTF-8 format).
 * \li \c T/F True if the pitch is set as ratio, False if in cents.
 * \li        Either:
 * \li \li \c h/d The numerator of the pitch ratio (relative to the note used).
 * \li \li \c h   The denominator of the pitch ratio if the numerator is an
 *                integer (h).
 * \li        Or:
 * \li \li \c d   The pitch difference in cents (relative to the note used).
 */


/**
 * Gets Note table information of the given Song.
 *
 * The following OSC argument is expected:
 *
 * \li \c i   The Song ID.
 *
 * The response consists of one call of <host_path>/note_table_info, zero or
 * more calls of <host_path>/note_info, zero or more calls of
 * <host_path>/note_mod_info and one call of <host_path>/notes_sent (to
 * indicate finished transmission of note table info).
 */
int Listener_get_note_table(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_NOTE_TABLE_H


