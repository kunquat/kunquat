

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_TUNING_TABLE_H
#define K_TUNING_TABLE_H


#include <containers/AAtree.h>
#include <kunquat/limits.h>
#include <mathnum/Real.h>
#include <string/Streader.h>

#include <stdlib.h>


/**
 * This object contains the tuning specification.
 */
typedef struct Tuning_table
{
    int note_count;
    int ref_note;
    int ref_note_retuned;
    double ref_pitch;
    double init_pitch_offset_cents;
    double pitch_offset;
    double pitch_offset_cents;
    Real octave_ratio;
    double oct_ratio_cents;
    Real oct_factors[KQT_TUNING_TABLE_OCTAVES];

    struct
    {
        double cents;
        Real ratio;
        Real ratio_retuned;
    } notes[KQT_TUNING_TABLE_NOTES];

    AAtree* pitch_map;
} Tuning_table;


#define TUNING_TABLE_DEFAULT_REF_PITCH 523.25113060119725
#define TUNING_TABLE_DEFAULT_OCTAVE_RATIO (Real_init_as_frac(REAL_AUTO, 2, 1))


/**
 * Create a new Tuning table.
 *
 * \param ref_pitch      The reference pitch -- must be > \c 0.
 * \param octave_ratio   The width of an octave -- must not be \c NULL and
 *                       must be greater than 1.
 *
 * \return   The new Tuning table if successful, or \c NULL if memory allocation
 *           fails.
 */
Tuning_table* new_Tuning_table(double ref_pitch, Real* octave_ratio);


/**
 * Create a Tuning table from a textual description.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Tuning table if successful, otherwise \c NULL.
 */
Tuning_table* new_Tuning_table_from_string(Streader* sr);


/**
 * Get the number of notes in the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The number of notes.
 */
int Tuning_table_get_note_count(Tuning_table* tt);


/**
 * Get the number of note modifiers in the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The number of note modifiers.
 */
int Tuning_table_get_note_mod_count(Tuning_table* tt);


/**
 * Set the reference note for the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the new reference note -- must be
 *                >= \c 0 and < \c KQT_SCALE_NOTES.
 *
 * \return   \c true if successful, or \c false if there is no note
 *           at \a index.
 */
bool Tuning_table_set_ref_note(Tuning_table* tt, int index);


/**
 * Get the initial reference note of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Tuning_table_get_ref_note(Tuning_table* tt);


/**
 * Get the current reference note of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Tuning_table_get_cur_ref_note(Tuning_table* tt);


/**
 * Set the reference pitch for the Tuning table.
 *
 * \param tt          The Tuning table -- must not be \c NULL.
 * \param ref_pitch   The reference pitch -- must be > \c 0.
 */
void Tuning_table_set_ref_pitch(Tuning_table* tt, double ref_pitch);


/**
 * Get the reference pitch of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The reference pitch.
 */
double Tuning_table_get_ref_pitch(Tuning_table* tt);


/**
 * Set the pitch offset of the Tuning table in cents.
 *
 * \param tt       The Tuning table -- must not be \c NULL.
 * \param offset   The pitch offset in cents -- must be finite.
 */
void Tuning_table_set_pitch_offset(Tuning_table* tt, double offset);


/**
 * Set the octave size as a ratio between adjacent octaves.
 *
 * \param tt             The Tuning table -- must not be \c NULL.
 * \param octave_ratio   The new ratio -- must not be \c NULL and must be
 *                       > \c 0.
 */
void Tuning_table_set_octave_ratio(Tuning_table* tt, Real* octave_ratio);


/**
 * Get the octave ratio of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The octave ratio.
 */
Real* Tuning_table_get_octave_ratio(Tuning_table* tt);


/**
 * Set the octave size in cents.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param cents   The new size in cents -- must be a finite value.
 */
void Tuning_table_set_octave_ratio_cents(Tuning_table* tt, double cents);


/**
 * Get the octave size in cents.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The size in cents if the ratio is defined in cents, otherwise
 *           \c NAN.
 */
double Tuning_table_get_octave_ratio_cents(Tuning_table* tt);


/**
 * Insert a new note into the Tuning table.
 *
 * All subsequent notes will be shifted forwards in the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set, or \c -1 if memory allocation
 *           failed. The index is never larger than \a index.
 */
int Tuning_table_ins_note(Tuning_table* tt, int index, Real* ratio);


/**
 * Insert a new note at the Tuning table using cents.
 *
 * All subsequent notes will be shifted forwards in the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set, or \c -1 if memory allocation
 *           failed. The index is never larger than \a index.
 */
int Tuning_table_ins_note_cents(Tuning_table* tt, int index, double cents);


/**
 * Get the (original) pitch ratio of a note in the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Tuning_table_get_note_ratio(Tuning_table* tt, int index);


/**
 * Get the current pitch ratio of a note in the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Tuning_table_get_cur_note_ratio(Tuning_table* tt, int index);


/**
 * Get the initial pitch ratio of a note in the Tuning table in cents.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Tuning_table_get_note_cents(Tuning_table* tt, int index);


/**
 * Get the current pitch ratio of a note in the Tuning table in cents.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_TUNING_TABLE_NOTES.
 *
 * \return   The ratio in cents if the note exists and the ratio is defined in
 *           cents, otherwise \c NAN.
 */
double Tuning_table_get_cur_note_cents(Tuning_table* tt, int index);


/**
 * Get the pitch of a note in the Tuning table.
 *
 * \param tt       The Tuning table -- must not be \c NULL.
 * \param index    The index of the note -- must be >= \c 0 and
 *                 < \c KQT_TUNING_TABLE_NOTES.
 * \param octave   The octave -- must be >= \c KQT_TUNING_TABLE_OCTAVE_FIRST
 *                 and <= \c KQT_TUNING_TABLE_OCTAVE_LAST.
 *
 * \return   The pitch if the note exists, otherwise a negative value.
 */
double Tuning_table_get_pitch(Tuning_table* tt, int index, int octave);


/**
 * Get the pitch of a note in the Tuning table based on cents.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param cents   Number of cents offset from 440 Hz -- must be finite.
 *
 * \return   The pitch, or a negative value if \a tt does not contain notes.
 */
double Tuning_table_get_pitch_from_cents(Tuning_table* tt, double cents);


/**
 * Retune the Tuning table.
 *
 * \param tt            The Tuning table -- must not be \c NULL.
 * \param new_ref       The new reference note -- must be
 *                      < \c KQT_SCALE_NOTES. If new_ref < \c 0, the tuning
 *                      will be reset to original.
 * \param fixed_point   A note whose frequency won't change in the retuning
 *                      process -- must be >= \c 0 and < \c KQT_TUNING_TABLE_NOTES.
 */
void Tuning_table_retune(Tuning_table* tt, int new_ref, int fixed_point);


/**
 * Retune the Tuning table with the initial parameters of another Tuning table.
 *
 * \param tt       The Tuning table to be retuned -- must not be \c NULL.
 * \param source   The source Tuning table used as the basis of the retuning -- must
 *                 not be \c NULL. The source Tuning table should have the same
 *                 number of notes as the Tuning table to be modified.
 *
 * \return   \c true if successful, or \c false if the Tuning tables contain
 *           different amounts of notes.
 */
bool Tuning_table_retune_with_source(Tuning_table* tt, Tuning_table* source);


/**
 * Estimate the current pitch drift in the Tuning table.
 * The estimate is most useful if the current tuning uses the same reference
 * note as the original tuning.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param drift   The Real object where the result is stored -- must not be
 *                \c NULL.
 *
 * \return   The parameter \a drift.
 */
Real* Tuning_table_drift(Tuning_table* tt, Real* drift);


/**
 * Clear the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 */
void Tuning_table_clear(Tuning_table* tt);


/**
 * Reset all transient parameters of the Tuning_table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 */
void Tuning_table_reset(Tuning_table* tt);


/**
 * Destroy the Tuning table.
 *
 * \param tt   The Tuning table, or \c NULL.
 */
void del_Tuning_table(Tuning_table* tt);


#endif // K_TUNING_TABLE_H


