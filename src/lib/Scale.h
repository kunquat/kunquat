

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


#ifndef K_SCALE_H
#define K_SCALE_H


#include <Real.h>
#include <pitch_t.h>
#include <kunquat/limits.h>
#include <File_base.h>
#include <File_tree.h>

#include <wchar.h>


/**
 * This object contains the tuning specification.
 */
typedef struct Scale
{
    int note_count;
    int ref_note;
    int ref_note_retuned;
    pitch_t ref_pitch;
    Real octave_ratio;
    double oct_ratio_cents;
    Real oct_factors[KQT_SCALE_OCTAVES];
    struct
    {
        double cents;
        Real ratio;
    } note_mods[KQT_SCALE_NOTE_MODS];
    struct
    {
        double cents;
        Real ratio;
        Real ratio_retuned;
    } notes[KQT_SCALE_NOTES];
} Scale;


/**
 * Creates a new Scale.
 *
 * The caller must eventually destroy the Scale with del_Scale().
 *
 * \param ref_pitch      The reference pitch -- must be > \c 0.
 * \param octave_ratio   The width of an octave -- must not be \c NULL and
 *                       must be greater than 0.
 *
 * \return   The new Scale if successful, or \c NULL if memory allocation
 *           fails.
 */
Scale* new_Scale(pitch_t ref_pitch, Real* octave_ratio);


/**
 * Reads a Scale from a File tree.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Scale_read(Scale* scale, File_tree* tree, Read_state* state);


/**
 * Gets the number of notes in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The number of notes.
 */
int Scale_get_note_count(Scale* scale);


/**
 * Gets the number of note modifiers in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The number of note modifiers.
 */
int Scale_get_note_mod_count(Scale* scale);


/**
 * Sets the reference note for the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the new reference note -- must be
 *                >= \c 0 and < \c KQT_SCALE_NOTES.
 *
 * \return   \c true if successful, or \c false if there is no note
 *           at \a index.
 */
bool Scale_set_ref_note(Scale* scale, int index);


/**
 * Gets the initial reference note of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Scale_get_ref_note(Scale* scale);


/**
 * Gets the current reference note of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Scale_get_cur_ref_note(Scale* scale);


/**
 * Sets the reference pitch for the Scale.
 *
 * \param scale       The Scale -- must not be \c NULL.
 * \param ref_pitch   The reference pitch -- must be > \c 0.
 */
void Scale_set_ref_pitch(Scale* scale, pitch_t ref_pitch);


/**
 * Gets the reference pitch of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The reference pitch.
 */
pitch_t Scale_get_ref_pitch(Scale* scale);


/**
 * Sets the octave size as a ratio between adjacent octaves.
 *
 * \param scale          The Scale -- must not be \c NULL.
 * \param octave_ratio   The new ratio -- must not be \c NULL and must be
 *                       > \c 0.
 */
void Scale_set_octave_ratio(Scale* scale, Real* octave_ratio);


/**
 * Gets the octave ratio of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The octave ratio.
 */
Real* Scale_get_octave_ratio(Scale* scale);


/**
 * Sets the octave size in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param cents   The new size in cents -- must be a finite value.
 */
void Scale_set_octave_ratio_cents(Scale* scale, double cents);


/**
 * Gets the octave size in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The size in cents if the ratio is defined in cents, otherwise
 *           \c NAN.
 */
double Scale_get_octave_ratio_cents(Scale* scale);


/**
 * Sets a new note at the Scale.
 * Any existing note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_set_note(Scale* scale,
                   int index,
                   Real* ratio);


/**
 * Sets a new note at the Scale using cents.
 * Any existing note at the target index will be replaced.
 * The note will be set at no further than the first unoccupied index.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_set_note_cents(Scale* scale,
                         int index,
                         double cents);


/**
 * Inserts a new note at the Scale.
 * All subsequent notes will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_ins_note(Scale* scale,
                   int index,
                   Real* ratio);


/**
 * Inserts a new note at the Scale using cents.
 * All subsequent notes will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_ins_note_cents(Scale* scale,
                         int index,
                         double cents);


/**
 * Deletes a note at the Scale.
 * All subsequent notes will be shifted backwards in the Scale.
 * If the target note doesn't exist, the Scale won't be modified.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be deleted -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 */
void Scale_del_note(Scale* scale, int index);


/**
 * Moves a note from one index to another in the Scale.
 *
 * \param scale     The Scale -- must not be NULL.
 * \param index     The index of the note to be moved -- must be >= 0 and
 *                  < KQT_SCALE_NOTES. If this index is empty, the scale
 *                  will remain unchanged.
 * \param new_index The destination index -- must be >= 0 and
 *                  < KQT_SCALE_NOTES. If this index is empty, the note
 *                  will be moved to the end of the note listing.
 *
 * \return   The actual destination index. This is never larger than
 *           \a new_index. If the note at \a index doesn't exist, \a index
 *           will be returned.
 */
int Scale_move_note(Scale* scale, int index, int new_index);


/**
 * Gets the (original) pitch ratio of a note in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Scale_get_note_ratio(Scale* scale, int index);


/**
 * Gets the current pitch ratio of a note in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Scale_get_cur_note_ratio(Scale* scale, int index);


/**
 * Gets the initial pitch ratio of a note in the Scale in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Scale_get_note_cents(Scale* scale, int index);


/**
 * Gets the current pitch ratio of a note in the Scale in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Scale_get_cur_note_cents(Scale* scale, int index);


/**
 * Sets a new note modifier at the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the modifier to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTE_MODS.
 * \param ratio   The pitch ratio between the unmodified note and note with
 *                this modifier -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_set_note_mod(Scale* scale,
                       int index,
                       Real* ratio);


/**
 * Sets a new note modifier at the Scale in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the modifier to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTE_MODS.
 * \param cents   The pitch ratio between the unmodified note and note with
 *                this modifier -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_set_note_mod_cents(Scale* scale,
                             int index,
                             double cents);


/**
 * Inserts a new note modifier at the Scale. All subsequent
 * note modifiers will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the modifier to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTE_MODS.
 * \param ratio   The pitch ratio between the unmodified note and note with
 *                this modifier -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_ins_note_mod(Scale* scale,
                       int index,
                       Real* ratio);


/**
 * Inserts a new note modifier at the Scale in cents. All subsequent
 * note modifiers will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the modifier to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTE_MODS.
 * \param cents   The pitch ratio between the unmodified note and note with
 *                this modifier -- must be a finite value.
 *
 * \return   The index that was actually set. This is never larger than
 *           \a index.
 */
int Scale_ins_note_mod_cents(Scale* scale,
                             int index,
                             double cents);


/**
 * Deletes a note modifier at the Scale. All subsequent
 * note modifiers will be shifted backwards in the Scale.
 *
 * \param scale   The Scale -- must not be NULL.
 * \param index   The index of the note modifier to be deleted -- must be
 *                >= \c 0 and < \c KQT_SCALE_NOTE_MODS.
 */
void Scale_del_note_mod(Scale* scale, int index);


/**
 * Moves a note modifier from one index to another in the Scale.
 *
 * \param scale       The Scale -- must not be \c NULL.
 * \param index       The index of the note modifier to be moved -- must be
 *                    >= \c 0 and < \c KQT_SCALE_NOTE_MODS.
 * \param new_index   The destination index -- must be >= \c 0 and
 *                    < \c KQT_SCALE_NOTE_MODS.
 *
 * \return   The actual destination index. This is never larger than
 *           \a new_index. If the note modifier at \a index doesn't exist,
 *           \a index will be returned.
 */
int Scale_move_note_mod(Scale* scale, int index, int new_index);


/**
 * Gets the pitch ratio of a note modifier in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note modifier -- must be >= \c 0
 *                and < \c KQT_SCALE_NOTE_MODS.
 *
 * \return   The ratio if the note modifier exists, otherwise \c NULL.
 */
Real* Scale_get_note_mod_ratio(Scale* scale, int index);


/**
 * Gets the pitch ratio of a note modifier in the Scale in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note modifier -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTE_MODS.
 *
 * \return   The ratio in cents if the note modifier exists and the ratio is
 *           defined in cents, otherwise \c NAN.
 */
double Scale_get_note_mod_cents(Scale* scale, int index);


/**
 * Gets the pitch of a note in the Scale.
 *
 * \param scale    The Scale -- must not be \c NULL.
 * \param index    The index of the note -- must be >= \c 0 and
 *                 < \c KQT_SCALE_NOTES.
 * \param mod      The note modifier -- must be < \c KQT_SCALE_NOTE_MODS.
 *                 Negative value means that no modifier will be used.
 * \param octave   The octave -- must be >= \c KQT_SCALE_OCTAVE_FIRST
 *                 and <= \c KQT_SCALE_OCTAVE_LAST.
 *
 * \return   The pitch if the note exists, otherwise a negative value.
 */
pitch_t Scale_get_pitch(Scale* scale,
                        int index,
                        int mod,
                        int octave);


/**
 * Retunes the Scale.
 *
 * \param scale         The Scale -- must not be \c NULL.
 * \param new_ref       The new reference note -- must be
 *                      < \c KQT_SCALE_NOTES. If new_ref < \c 0, the tuning
 *                      will be reset to original.
 * \param fixed_point   A note whose frequency won't change in the retuning
 *                      process -- must be >= \c 0 and < \c KQT_SCALE_NOTES.
 */
void Scale_retune(Scale* scale, int new_ref, int fixed_point);


/**
 * Estimates the current pitch drift in the Scale.
 * The estimate is most useful if the current tuning uses the same reference
 * note as the original tuning.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param drift   The Real object where the result is stored -- must not be
 *                \c NULL.
 *
 * \return   The parameter \a drift.
 */
Real* Scale_drift(Scale* scale, Real* drift);


/**
 * Clears the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 */
void Scale_clear(Scale* scale);


/**
 * Destroys the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 */
void del_Scale(Scale* scale);


#endif // KQT_SCALE_H


