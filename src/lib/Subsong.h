

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#ifndef K_SUBSONG_H
#define K_SUBSONG_H


#include <stdint.h>

#include <File_base.h>


#define KQT_SECTION_NONE (-1)


/**
 * Subsong specifies some initial playback settings and the order in which
 * Patterns are played.
 */
typedef struct Subsong
{
    double tempo;      ///< Initial tempo.
    double global_vol; ///< Initial global volume.
    int scale;         ///< Index of the initial Scale.
    int res;           ///< Size reserved for the section list.
    int16_t* pats;     ///< Section list that contains the Pattern numbers.
} Subsong;


#define SUBSONG_DEFAULT_TEMPO (120)
#define SUBSONG_DEFAULT_GLOBAL_VOL (-4)
#define SUBSONG_DEFAULT_SCALE (0)


/**
 * Creates a new Subsong.
 *
 * \return   The new Subsong if successful, or \c NULL if memory allocation
 *           failed.
 */
Subsong* new_Subsong(void);


/**
 * Creates a new Subsong from a textual description.
 *
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   The new Subsong if successful, otherwise \c NULL. \a state
 *           will _not_ be updated if memory allocation failed.
 */
Subsong* new_Subsong_from_string(char* str, Read_state* state);


/**
 * Sets the pattern for the specified Subsong position.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and < \c KQT_SECTIONS_MAX.
 * \param pat     The pattern number -- must be >= \c 0 or KQT_SECTION_NONE.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Subsong_set(Subsong* ss, int index, int16_t pat);


/**
 * Gets the pattern from the specified Subsong position.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and < \c KQT_SECTIONS_MAX.
 *
 * \return   The pattern number if one exists, otherwise KQT_SECTION_NONE.
 */
int16_t Subsong_get(Subsong* ss, int index);


/**
 * Gets the length of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The length.
 */
int16_t Subsong_get_length(Subsong* ss);


/**
 * Sets the initial tempo of the Subsong.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param tempo   The tempo -- must be finite and positive.
 */
void Subsong_set_tempo(Subsong* ss, double tempo);


/**
 * Gets the initial tempo of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The tempo.
 */
double Subsong_get_tempo(Subsong* ss);


/**
 * Sets the initial global volume of the Subsong.
 *
 * \param ss    The Subsong -- must not be \c NULL.
 * \param vol   The global volume -- must be finite or \c -INFINITY.
 */
void Subsong_set_global_vol(Subsong* ss, double vol);


/**
 * Gets the initial global volume of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The global volume.
 */
double Subsong_get_global_vol(Subsong* ss);


/**
 * Sets the initial default Scale of the Subsong.
 *
 * \param ss      The Subsong -- must not be \c NULL.
 * \param index   The Scale index -- must be >= \c 0 and
 *                < \c KQT_SCALES_MAX.
 */
void Subsong_set_scale(Subsong* ss, int index);


/**
 * Gets the initial default Scale of the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 *
 * \return   The Scale index.
 */
int Subsong_get_scale(Subsong* ss);


/**
 * Clears the Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 */
// void Subsong_clear(Subsong* ss);


/**
 * Destroys an existing Subsong.
 *
 * \param ss   The Subsong -- must not be \c NULL.
 */
void del_Subsong(Subsong* ss);


#endif // K_SUBSONG_H


