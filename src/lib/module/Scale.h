

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SCALE_H
#define K_SCALE_H


#include <containers/AAtree.h>
#include <kunquat/limits.h>
#include <mathnum/Real.h>
#include <string/Streader.h>


/**
 * This object contains the tuning specification.
 */
typedef struct Scale
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
    Real oct_factors[KQT_SCALE_OCTAVES];
    struct
    {
        double cents;
        Real ratio;
        Real ratio_retuned;
    } notes[KQT_SCALE_NOTES];
    AAtree* pitch_map;
} Scale;


#define SCALE_DEFAULT_REF_PITCH 523.25113060119725
#define SCALE_DEFAULT_OCTAVE_RATIO (Real_init_as_frac(REAL_AUTO, 2, 1))


/**
 * Create a new Scale.
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
Scale* new_Scale(double ref_pitch, Real* octave_ratio);


/**
 * Create a Scale from a textual description.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Scale if successful, otherwise \c NULL.
 */
Scale* new_Scale_from_string(Streader* sr);


/**
 * Get the number of notes in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The number of notes.
 */
int Scale_get_note_count(Scale* scale);


/**
 * Get the number of note modifiers in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The number of note modifiers.
 */
int Scale_get_note_mod_count(Scale* scale);


/**
 * Set the reference note for the Scale.
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
 * Get the initial reference note of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Scale_get_ref_note(Scale* scale);


/**
 * Get the current reference note of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The index of the reference note.
 */
int Scale_get_cur_ref_note(Scale* scale);


/**
 * Set the reference pitch for the Scale.
 *
 * \param scale       The Scale -- must not be \c NULL.
 * \param ref_pitch   The reference pitch -- must be > \c 0.
 */
void Scale_set_ref_pitch(Scale* scale, double ref_pitch);


/**
 * Get the reference pitch of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The reference pitch.
 */
double Scale_get_ref_pitch(Scale* scale);


/**
 * Set the pitch offset of the Scale in cents.
 *
 * \param scale    The Scale -- must not be \c NULL.
 * \param offset   The pitch offset in cents -- must be finite.
 */
void Scale_set_pitch_offset(Scale* scale, double offset);


/**
 * Set the octave size as a ratio between adjacent octaves.
 *
 * \param scale          The Scale -- must not be \c NULL.
 * \param octave_ratio   The new ratio -- must not be \c NULL and must be
 *                       > \c 0.
 */
void Scale_set_octave_ratio(Scale* scale, Real* octave_ratio);


/**
 * Get the octave ratio of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The octave ratio.
 */
Real* Scale_get_octave_ratio(Scale* scale);


/**
 * Set the octave size in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param cents   The new size in cents -- must be a finite value.
 */
void Scale_set_octave_ratio_cents(Scale* scale, double cents);


/**
 * Get the octave size in cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 *
 * \return   The size in cents if the ratio is defined in cents, otherwise
 *           \c NAN.
 */
double Scale_get_octave_ratio_cents(Scale* scale);


/**
 * Insert a new note at the Scale.
 * All subsequent notes will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param ratio   The pitch ratio between the new note and reference pitch
 *                -- must not be \c NULL and must be > \c 0.
 *
 * \return   The index that was actually set, or \c -1 if memory allocation
 *           failed. The index is never larger than \a index.
 */
int Scale_ins_note(Scale* scale, int index, Real* ratio);


/**
 * Insert a new note at the Scale using cents.
 * All subsequent notes will be shifted forwards in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note to be set -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 * \param cents   The pitch ratio between the new note and reference pitch
 *                in cents -- must be a finite value.
 *
 * \return   The index that was actually set, or \c -1 if memory allocation
 *           failed. The index is never larger than \a index.
 */
int Scale_ins_note_cents(Scale* scale, int index, double cents);


/**
 * Get the (original) pitch ratio of a note in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Scale_get_note_ratio(Scale* scale, int index);


/**
 * Get the current pitch ratio of a note in the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param index   The index of the note -- must be >= \c 0 and
 *                < \c KQT_SCALE_NOTES.
 *
 * \return   The ratio if the note exists, otherwise \c NULL.
 */
Real* Scale_get_cur_note_ratio(Scale* scale, int index);


/**
 * Get the initial pitch ratio of a note in the Scale in cents.
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
 * Get the current pitch ratio of a note in the Scale in cents.
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
 * Get the pitch of a note in the Scale.
 *
 * \param scale    The Scale -- must not be \c NULL.
 * \param index    The index of the note -- must be >= \c 0 and
 *                 < \c KQT_SCALE_NOTES.
 * \param octave   The octave -- must be >= \c KQT_SCALE_OCTAVE_FIRST
 *                 and <= \c KQT_SCALE_OCTAVE_LAST.
 *
 * \return   The pitch if the note exists, otherwise a negative value.
 */
double Scale_get_pitch(Scale* scale, int index, int octave);


/**
 * Get the pitch of a note in the Scale based on cents.
 *
 * \param scale   The Scale -- must not be \c NULL.
 * \param cents   Number of cents centered to 440 Hz -- must be finite.
 *
 * \return   The pitch, or a negative value if \a scale doesn't contain notes.
 */
double Scale_get_pitch_from_cents(Scale* scale, double cents);


/**
 * Retune the Scale.
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
 * Retune the Scale with the initial parameters of another Scale.
 *
 * \param scale    The Scale to be retuned -- must not be \c NULL.
 * \param source   The source Scale used as the basis of the retuning -- must
 *                 not be \c NULL. The source Scale should have the same
 *                 number of notes as the Scale to be modified.
 *
 * \return   \c true if successful, or \c false if the Scales contain
 *           different amounts of notes.
 */
bool Scale_retune_with_source(Scale* scale, Scale* source);


/**
 * Estimate the current pitch drift in the Scale.
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
 * Clear the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 */
void Scale_clear(Scale* scale);


/**
 * Reset all transient parameters of the Scale.
 *
 * \param scale   The Scale -- must not be \c NULL.
 */
void Scale_reset(Scale* scale);


/**
 * Destroy the Scale.
 *
 * \param scale   The Scale, or \c NULL.
 */
void del_Scale(Scale* scale);


#endif // KQT_SCALE_H


