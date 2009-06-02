

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


#ifndef K_NOTE_TABLE_H
#define K_NOTE_TABLE_H


#include <Real.h>
#include <pitch_t.h>
#include <Song_limits.h>
#include <File_base.h>
#include <File_tree.h>

#include <wchar.h>


/**
 * This object contains the tuning specification.
 */
typedef struct Note_table
{
    wchar_t name[NOTE_TABLE_NAME_MAX];
    int note_count;
    int ref_note;
    int ref_note_retuned;
    pitch_t ref_pitch;
    Real octave_ratio;
    double oct_ratio_cents;
    Real oct_factors[NOTE_TABLE_OCTAVES];
    struct
    {
        wchar_t name[NOTE_TABLE_NOTE_MOD_NAME_MAX];
        double cents;
        Real ratio;
    } note_mods[NOTE_TABLE_NOTE_MODS];
    struct
    {
        wchar_t name[NOTE_TABLE_NOTE_NAME_MAX];
        double cents;
        Real ratio;
        Real ratio_retuned;
    } notes[NOTE_TABLE_NOTES];
} Note_table;


/**
 * Creates a new Note table.
 *
 * The caller must eventually destroy the table with del_Note_table().
 *
 * \param name           The name of the table (this will be copied).
 * \param ref_pitch      The reference pitch -- must be > \c 0.
 * \param octave_ratio   The width of an octave -- must not be \c NULL and
 *                       must be greater than 0.
 *
 * \return   The new Note table if successful, or \c NULL if memory allocation
 *           fails.
 */
Note_table* new_Note_table(wchar_t* name, pitch_t ref_pitch, Real* octave_ratio);


/**
 * Reads a Note table from a File tree.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Note_table_read(Note_table* table, File_tree* tree, Read_state* state);


/**
 * Sets a name for the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param name    The name.
 */
void Note_table_set_name(Note_table* table, wchar_t* name);


/**
 * Gets the name of the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The name.
 */
wchar_t* Note_table_get_name(Note_table* table);


/**
 * Gets the number of notes in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The number of notes.
 */
int Note_table_get_note_count(Note_table* table);


/**
 * Gets the number of note modifiers in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The number of note modifiers.
 */
int Note_table_get_note_mod_count(Note_table* table);


/**
 * Sets the reference note for the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the new reference note -- must be
 *                >= \c 0 and < \c NOTE_TABLE_NOTES.
 *
 * \return   \c true if successful, or \c false if there is no note
 *           at \a index.
 */
bool Note_table_set_ref_note(Note_table* table, int index);


/**
 * Gets the initial reference note of the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Note_table_get_ref_note(Note_table* table);


/**
 * Gets the current reference note of the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Note_table_get_cur_ref_note(Note_table* table);


/**
 * Sets the reference pitch for the Note table.
 *
 * \param table       The Note table -- must not be \c NULL.
 * \param ref_pitch   The reference pitch -- must be > \c 0.
 */
void Note_table_set_ref_pitch(Note_table* table, pitch_t ref_pitch);


/**
 * Gets the reference pitch of the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The reference pitch.
 */
pitch_t Note_table_get_ref_pitch(Note_table* table);


/**
 * Sets the octave size as a ratio between adjacent octaves.
 *
 * \param table          The Note table -- must not be \c NULL.
 * \param octave_ratio   The new ratio -- must not be \c NULL and must be
 *                       > \c 0.
 */
void Note_table_set_octave_ratio(Note_table* table, Real* octave_ratio);


/**
 * Gets the octave ratio of the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The octave ratio.
 */
Real* Note_table_get_octave_ratio(Note_table* table);


/**
 * Sets the octave size in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param cents   The new size in cents -- must be a finite value.
 */
void Note_table_set_octave_ratio_cents(Note_table* table, double cents);


/**
 * Gets the octave size in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 *
 * \return   The size in cents if the ratio is defined in cents, otherwise
 *           \c NAN.
 */
double Note_table_get_octave_ratio_cents(Note_table* table);


/**
 * Sets a new note at the Note table.
 * Any existent note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 * \param name    The name of the note -- must not be \c NULL or empty string.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_set_note(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio);


/**
 * Sets a new note at the Note table using cents.
 * Any existent note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 * \param name    The name of the note -- must not be \c NULL or empty string.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_set_note_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents);


/**
 * Inserts a new note at the Note table.
 * All subsequent notes will be shifted forward in the table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 * \param name    The name of the note -- must not be \c NULL or empty string.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_ins_note(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio);


/**
 * Inserts a new note at the Note table using cents.
 * All subsequent notes will be shifted forward in the table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 * \param name    The name of the note -- must not be \c NULL or empty string.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_ins_note_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents);


/**
 * Deletes a note at the Note table.
 * All subsequent notes will be shifted backward in the table.
 * If the target note doesn't exist, the table won't be modified.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be deleted -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 */
void Note_table_del_note(Note_table* table, int index);


/**
 * Moves a note from one index to another in the Note table.
 *
 * \param table     The Note table -- must not be NULL.
 * \param index     The index of the note to be moved -- must be >= 0 and
 *                  < NOTE_TABLE_NOTES. If this index is empty, the table
 *                  will remain unchanged.
 * \param new_index The destination index -- must be >= 0 and
 *                  < NOTE_TABLE_NOTES. If this index is empty, the note
 *                  will be moved to the end of the note listing.
 *
 * \return   The actual destination index. This is never larger than
 *           \a new_index. If the note at \a index doesn't exist, \a index
 *           will be returned.
 */
int Note_table_move_note(Note_table* table, int index, int new_index);


/**
 * Gets the name of a note in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 *
 * \return   The name if the note exists, otherwise \c NULL.
 */
wchar_t* Note_table_get_note_name(Note_table* table, int index);


/**
 * Gets the (original) pitch ratio of a note in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Note_table_get_note_ratio(Note_table* table, int index);


/**
 * Gets the current pitch ratio of a note in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Note_table_get_cur_note_ratio(Note_table* table, int index);


/**
 * Gets the initial pitch ratio of a note in the Note table in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Note_table_get_note_cents(Note_table* table, int index);


/**
 * Gets the current pitch ratio of a note in the Note table in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Note_table_get_cur_note_cents(Note_table* table, int index);


/**
 * Sets a new note modifier at the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 * \param name    The name of the note modifier -- must not be \c NULL or
 *                empty string.
 * \param ratio   The pitch ratio between the unmodified note and note with
 *                this modifier -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_set_note_mod(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio);


/**
 * Sets a new note modifier at the Note table in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 * \param name    The name of the note modifier -- must not be \c NULL or
 *                empty string.
 * \param cents   The pitch ratio between the unmodified note and note with
 *                this modifier -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_set_note_mod_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents);


/**
 * Inserts a new note modifier at the Note table. All subsequent
 * note modifiers will be shifted forward in the table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 * \param name    The name of the note modifier -- must not be \c NULL.
 * \param ratio   The pitch ratio between the unmodified note and note with
 *                this modifier -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_ins_note_mod(Note_table* table,
        int index,
        wchar_t* name,
        Real* ratio);


/**
 * Inserts a new note modifier at the Note table in cents. All subsequent
 * note modifiers will be shifted forward in the table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the table to be set -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 * \param name    The name of the note modifier -- must not be \c NULL.
 * \param cents   The pitch ratio between the unmodified note and note with
 *                this modifier -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Note_table_ins_note_mod_cents(Note_table* table,
        int index,
        wchar_t* name,
        double cents);


/**
 * Deletes a note modifier at the Note table. All subsequent
 * note modifiers will be shifted backward in the table.
 *
 * \param table   The Note table -- must not be NULL.
 * \param index   The index of the table to be deleted -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 */
void Note_table_del_note_mod(Note_table* table, int index);


/**
 * Moves a note modifier from one index to another in the Note table.
 *
 * \param table       The Note table -- must not be \c NULL.
 * \param index       The index of the note modifier to be moved -- must be
 *                    >= \c 0 and < \c NOTE_TABLE_NOTE_MODS.
 * \param new_index   The destination index -- must be >= \c 0 and
 *                    < \c NOTE_TABLE_NOTE_MODS.
 *
 * \return   The actual destination index. This is never larger than
 *           \a new_index. If the note modifier at \a index doesn't exist,
 *           \a index will be returned.
 */
int Note_table_move_note_mod(Note_table* table, int index, int new_index);


/**
 * Gets the name of a note modifier in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note modifier -- must be >= \c 0
 *                and < \c NOTE_TABLE_NOTE_MODS.
 *
 * \return   The name if the note modifier exists, otherwise \c NULL.
 */
wchar_t* Note_table_get_note_mod_name(Note_table* table, int index);


/**
 * Gets the pitch ratio of a note modifier in the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note modifier -- must be >= \c 0
 *                and < \c NOTE_TABLE_NOTE_MODS.
 *
 * \return   The ratio if the note modifier exists, otherwise \c NULL.
 */
Real* Note_table_get_note_mod_ratio(Note_table* table, int index);


/**
 * Gets the pitch ratio of a note modifier in the Note table in cents.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param index   The index of the note modifier -- must be >= \c 0 and
 *                < \c NOTE_TABLE_NOTE_MODS.
 *
 * \return   The ratio in cents if the note modifier exists and the ratio is
 *           defined in cents, otherwise \c NAN.
 */
double Note_table_get_note_mod_cents(Note_table* table, int index);


/**
 * Gets the pitch of a note in the Note table.
 *
 * \param table    The Note table -- must not be \c NULL.
 * \param index    The index of the note -- must be >= \c 0 and
 *                 < \c NOTE_TABLE_NOTES.
 * \param mod      The note modifier -- must be < \c NOTE_TABLE_NOTE_MODS.
 *                 Negative value means that no modifier will be used.
 * \param octave   The octave -- must be >= \c NOTE_TABLE_OCTAVE_FIRST
 *                 and <= \c NOTE_TABLE_OCTAVE_LAST.
 *
 * \return   The pitch if the note exists, otherwise a negative value.
 */
pitch_t Note_table_get_pitch(Note_table* table,
        int index,
        int mod,
        int octave);


/**
 * Retunes the Note table.
 *
 * \param table         The Note table -- must not be \c NULL.
 * \param new_ref       The new reference note -- must be
 *                      < \c NOTE_TABLE_NOTES. If new_ref < \c 0, the tuning
 *                      will be reset to original.
 * \param fixed_point   A note whose frequency won't change in the retuning
 *                      process -- must be >= \c 0 and < \c NOTE_TABLE_NOTES.
 */
void Note_table_retune(Note_table* table, int new_ref, int fixed_point);


/**
 * Estimates the current pitch drift in the Note table.
 * The estimate is most useful if the current tuning uses the same reference
 * note as the original tuning.
 *
 * \param table   The Note table -- must not be \c NULL.
 * \param drift   The Real object where the result is stored -- must not be
 *                \c NULL.
 *
 * \return   The parameter \a drift.
 */
Real* Note_table_drift(Note_table* table, Real* drift);


/**
 * Clears the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 */
void Note_table_clear(Note_table* table);


/**
 * Destroys the Note table.
 *
 * \param table   The Note table -- must not be \c NULL.
 */
void del_Note_table(Note_table* table);


#endif // K_NOTE_TABLE_H


