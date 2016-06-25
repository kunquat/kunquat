

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


#ifndef KQT_SONG_TABLE_H
#define KQT_SONG_TABLE_H


#include <init/sheet/Song.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * Song table contains the Songs.
 */
typedef struct Song_table Song_table;


/**
 * Create a new Song table.
 *
 * \return   The new Song table if successful, or \c NULL if memory
 *           allocation failed.
 */
Song_table* new_Song_table(void);


/**
 * Set a Song in the Song table.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 * \param song    The Song -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_table_set(Song_table* table, uint16_t index, Song* song);


/**
 * Get a Song from the Song table.
 *
 * Note: Songs after an empty index are considered hidden and are not
 * returned.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 *
 * \return   The Song if one exists, otherwise \c NULL.
 */
Song* Song_table_get(Song_table* table, uint16_t index);


/**
 * Set existent status of a Song.
 *
 * \param table      The Song table -- must not be \c NULL.
 * \param index      The song number -- must be >= \c 0 and
 *                   < \c KQT_SONGS_MAX.
 * \param existent   The new existent status.
 */
void Song_table_set_existent(Song_table* table, uint16_t index, bool existent);


/**
 * Get existent status of a Song.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 *
 * \return   The existent status.
 */
bool Song_table_get_existent(Song_table* table, uint16_t index);


/**
 * Destroy an existing Song table.
 *
 * \param table   The Song table, or \c NULL.
 */
void del_Song_table(Song_table* table);


#endif // KQT_SONG_TABLE_H


