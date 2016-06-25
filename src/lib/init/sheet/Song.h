

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


#ifndef KQT_SONG_H
#define KQT_SONG_H


#include <decl.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Song specifies some initial playback settings and the order in which
 * Patterns are played.
 */


/**
 * Create a new Song.
 *
 * \return   The new Song if succesongful, or \c NULL if memory allocation
 *           failed.
 */
Song* new_Song(void);


/**
 * Read the initial tempo of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param sr     The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Song_read_tempo(Song* song, Streader* sr);


/**
 * Get the initial tempo of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The tempo.
 */
double Song_get_tempo(const Song* song);


/**
 * Read the initial global volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param sr     The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Song_read_global_vol(Song* song, Streader* sr);


/**
 * Get the initial global volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The global volume.
 */
double Song_get_global_vol(const Song* song);


/**
 * Destroy an existing Song.
 *
 * \param song   The Song, or \c NULL.
 */
void del_Song(Song* song);


#endif // KQT_SONG_H


