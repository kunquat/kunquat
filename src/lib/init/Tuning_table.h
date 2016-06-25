

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


#ifndef KQT_TUNING_TABLE_H
#define KQT_TUNING_TABLE_H


#include <containers/AAtree.h>
#include <decl.h>
#include <kunquat/limits.h>
#include <string/Streader.h>

#include <stdlib.h>


/**
 * An adaptive tuning specification that can be retuned to match changing keys.
 */
struct Tuning_table
{
    int note_count;
    int ref_note;
    double ref_pitch;
    double global_offset;
    int center_octave;

    double octave_width;
    double octave_offsets[KQT_TUNING_TABLE_OCTAVES];

    double note_offsets[KQT_TUNING_TABLE_NOTES_MAX];

    AAtree* pitch_map;
};


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
int Tuning_table_get_note_count(const Tuning_table* tt);


/**
 * Get the initial reference note of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Tuning_table_get_ref_note(const Tuning_table* tt);


/**
 * Get the reference pitch of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The reference pitch.
 */
double Tuning_table_get_ref_pitch(const Tuning_table* tt);


/**
 * Get the global pitch offset of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The global pitch offset.
 */
double Tuning_table_get_global_offset(const Tuning_table* tt);


/**
 * Get the octave width of the Tuning table.
 *
 * \param tt   The Tuning table -- must not be \c NULL.
 *
 * \return   The octave ratio.
 */
double Tuning_table_get_octave_width(const Tuning_table* tt);


/**
 * Get the pitch offset of a note in the Tuning table.
 *
 * \param tt      The Tuning table -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \a Tuning_table_get_note_count(\a tt).
 *
 * \return   The pitch offset.
 */
double Tuning_table_get_pitch_offset(const Tuning_table* tt, int index);


/**
 * Get the note index nearest to a pitch.
 *
 * \param tt      The Tuning table -- must not be \c NULL or empty.
 * \param cents   The pitch -- must be finite.
 *
 * \return   The nearest note index.
 */
int Tuning_table_get_nearest_note_index(const Tuning_table* tt, double cents);


/**
 * Destroy the Tuning table.
 *
 * \param tt   The Tuning table, or \c NULL.
 */
void del_Tuning_table(Tuning_table* tt);


#endif // KQT_TUNING_TABLE_H


