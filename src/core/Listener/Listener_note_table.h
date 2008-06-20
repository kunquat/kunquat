

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


#include "Listener.h"


/**
 * The response method <host_path>/note_table_info contains the following
 * arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number.
 * \li \c s   The name of the Note table (in UTF-8 format).
 * \li \c i   The number of notes in the Note table.
 * \li \c i   The number of note modifiers in the Note table.
 * \li \c i   The initial reference note.
 * \li \c i   The current reference note.
 * \li \c d   The reference pitch.
 * \li \c T/F True if the octave size is set as ratio, False if in cents.
 * \li        Either:
 * \li \li \c h/d The numerator of the octave ratio.
 * \li \li \c h   The denominator of the octave ratio if the numerator is an
 *                integer (h).
 * \li        Or:
 * \li \li \c d   The octave ratio in cents.
 */

/**
 * The response method <host_path>/note_info contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number.
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
 * \li \c i   The Note table number.
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
 * The response method <host_path>/notes_sent is sent after sending all the
 * information of a Note table. This is the only response if the Note table
 * doesn't exist. The method contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number.
 */


/**
 * Gets Note table information of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 *
 * The response consists of at most one call of <host_path>/note_table_info,
 * zero or more calls of <host_path>/note_info, zero or more calls of
 * <host_path>/note_mod_info and one call of <host_path>/notes_sent (to
 * indicate finished transmission of note table info).
 */
Listener_callback Listener_get_note_table;


/**
 * Sets the name of a Note table.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c s   The new name (in UTF-8 format).
 */
Listener_callback Listener_set_note_table_name;


/**
 * Sets the reference note of a Note table.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c i   The index of the new reference note.
 */
Listener_callback Listener_set_note_table_ref_note;


/**
 * Sets the reference pitch of a Note table.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c d   The new reference pitch (> 0).
 */
Listener_callback Listener_set_note_table_ref_pitch;


/**
 * Sets the octave ratio of a Note table.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c T/F True if the octave size is a ratio, False if it is in cents.
 * 
 * If the size is a ratio, it can be a fraction or a float. If it is a
 * fraction:
 *
 * \li \c h   The numerator of the fraction.
 * \li \c h   The denominator of the fraction.
 *
 * Or, if the ratio is a float:
 *
 * \li \c d   The ratio.
 *
 * Or, if the octave size is in cents:
 *
 * \li \c d   The octave size in cents.
 */
Listener_callback Listener_set_note_table_octave_ratio;


/**
 * Sets the name of a note.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c i   The note number.
 * \li \c s   The new name (in UTF-8 format).
 *
 * If a new note is created, it will always be placed into the first
 * unoccupied slot in the Note table.
 */
Listener_callback Listener_set_note_name;


/**
 * Sets the ratio of a note.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c i   The note number.
 * \li \c T/F True if the note ratio is a ratio, False if it is in cents.
 * 
 * If the size is a ratio, it can be a fraction or a float. If it is a
 * fraction:
 *
 * \li \c h   The numerator of the fraction.
 * \li \c h   The denominator of the fraction.
 *
 * Or, if the ratio is a float:
 *
 * \li \c d   The ratio.
 *
 * Or, if the note ratio is in cents:
 *
 * \li \c d   The note ratio in cents.
 */
Listener_callback Listener_set_note_ratio;


/**
 * Removes a note from a Note table. Notes positioned after will be shifted
 * up.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c i   The note number.
 */
Listener_callback Listener_del_note;


/**
 * Inserts a blank note into a Note table. Notes positioned at the number and
 * after will be shifted down.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 * \li \c i   The note number.
 */
Listener_callback Listener_ins_note;


/**
 * Removes a Note table.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Note table number (0..16).
 */
Listener_callback Listener_del_note_table;


#endif // K_LISTENER_NOTE_TABLE_H


