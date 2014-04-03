

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent posongible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SONG_H
#define K_SONG_H


#include <stdint.h>

#include <string/Streader.h>


#define KQT_SECTION_NONE (-1)


/**
 * Ssong specifies some initial playback settings and the order in which
 * Patterns are played.
 */
typedef struct Song
{
    double tempo;      ///< Initial tempo.
    double global_vol; ///< Initial global volume.
    int scale;         ///< Index of the initial Scale.
    int res;           ///< Size reserved for the section list.
    int16_t* pats;     ///< Section list that contains the Pattern numbers.
} Song;


#define SONG_DEFAULT_TEMPO (120)
#define SONG_DEFAULT_GLOBAL_VOL (-4)
#define SONG_DEFAULT_SCALE (0)


/**
 * Creates a new Song.
 *
 * \return   The new Song if succesongful, or \c NULL if memory allocation
 *           failed.
 */
Song* new_Song(void);


/**
 * Creates a new Song from a textual description.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Song if succesongful, otherwise \c NULL.
 */
Song* new_Song_from_string(Streader* sr);


/**
 * Gets the length of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The length.
 */
int16_t Song_get_length(Song* song);


/**
 * Sets the initial tempo of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param tempo   The tempo -- must be finite and positive.
 */
void Song_set_tempo(Song* song, double tempo);


/**
 * Gets the initial tempo of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The tempo.
 */
double Song_get_tempo(Song* song);


/**
 * Sets the initial global volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param vol    The global volume -- must be finite or \c -INFINITY.
 */
void Song_set_global_vol(Song* song, double vol);


/**
 * Gets the initial global volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The global volume.
 */
double Song_get_global_vol(Song* song);


/**
 * Sets the initial default Scale of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Scale index -- must be >= \c 0 and
 *                < \c KQT_SCALES_MAX.
 */
void Song_set_scale(Song* song, int index);


/**
 * Gets the initial default Scale of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Scale index.
 */
int Song_get_scale(Song* song);


/**
 * Destroys an existing Song.
 *
 * \param song   The Song, or \c NULL.
 */
void del_Song(Song* song);


#endif // K_SONG_H


