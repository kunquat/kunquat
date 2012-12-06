

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SONG_H
#define K_SONG_H


#include <stdint.h>

#include <Bind.h>
#include <Connections.h>
#include <Device.h>
#include <Environment.h>
#include <kunquat/limits.h>
#include <frame.h>
#include <Subsong_table.h>
#include <Pat_table.h>
#include <Effect_table.h>
#include <Ins_table.h>
#include <Order_list.h>
#include <Random.h>
#include <Scale.h>
#include <Playdata.h>
#include <File_base.h>
#include <Event_handler.h>


typedef struct Song
{
    Device parent;
    uint64_t random_seed;               ///< The master random seed of the composition.
    Random* random;                     ///< The source for random data in the composition.
    Subsong_table* subsongs;            ///< The Subsongs.
    Order_list* order_lists[KQT_SONGS_MAX]; ///< Order lists.
    Pat_table* pats;                    ///< The Patterns.
    Ins_table* insts;                   ///< The Instruments.
    Effect_table* effects;              ///< The global Effects.
    Connections* connections;           ///< Device connections.
    Scale* scales[KQT_SCALES_MAX];      ///< The Scales.
    double mix_vol_dB;                  ///< Mixing volume in dB.
    double mix_vol;                     ///< Mixing volume.
    uint16_t init_subsong;              ///< Initial subsong number.
    Playdata* play_state;               ///< Playback state.
    Event_handler* event_handler;       ///< The Event handler.
    Playdata* skip_state;               ///< Skip state (used for length calculation).
    Channel* channels[KQT_COLUMNS_MAX]; ///< The channels used.
    Event_handler* skip_handler;        ///< Skip state Event handler.
    Environment* env;                   ///< Environment variables.
    Bind* bind;
} Song;


#define SONG_DEFAULT_BUF_COUNT (2)
#define SONG_DEFAULT_MIX_VOL (-8)
#define SONG_DEFAULT_INIT_SUBSONG (0)


/**
 * Creates a new Song.
 *
 * The caller shall eventually call del_Song() to destroy the Song returned.
 *
 * \param buf_size    Size of the mixing buffer -- must be > \c 0 and
 *                    <= KQT_BUFFER_SIZE_MAX.
 *
 * \see del_Song()
 *
 * \return   The new Song if successful, or \c NULL if memory allocation
 *           failed.
 */
Song* new_Song(uint32_t buf_size);


/**
 * Parses the composition header of a Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Song_parse_composition(Song* song, char* str, Read_state* state);


/**
 * Parses the random seed of the composition.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Song_parse_random_seed(Song* song, char* str, Read_state* state);


/**
 * Mixes a portion of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param nframes   The amount of frames to be mixed.
 * \param eh        The Event handler -- must not be \c NULL.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes.
 */
uint32_t Song_mix(Song* song, uint32_t nframes, Event_handler* eh);


/**
 * Skips part of the Song.
 *
 * \param song     The Song -- must not be \c NULL.
 * \param eh       The Event handler -- must not be \c NULL.
 * \param amount   The amount of frames to be skipped.
 *
 * \return   The amount of frames actually skipped. This is <= \a amount.
 */
uint64_t Song_skip(Song* song, Event_handler* eh, uint64_t amount);


/**
 * Sets the mixing volume of the Song.
 *
 * \param song      The Song -- must not be \c NULL.
 * \param mix_vol   The volume -- must be finite or -INFINITY.
 */
void Song_set_mix_vol(Song* song, double mix_vol);


/**
 * Gets the mixing volume of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The mixing volume.
 */
double Song_get_mix_vol(Song* song);


/**
 * Sets the initial subsong of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param num    The subsong number -- must be < \c KQT_SONGS_MAX.
 */
void Song_set_subsong(Song* song, uint16_t num);


/**
 * Gets the initial subsong of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The initial subsong number.
 */
uint16_t Song_get_subsong(Song* song);


/**
 * Gets the Subsong table from the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Subsong table.
 */
Subsong_table* Song_get_subsongs(Song* song);


/**
 * Gets the Patterns of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Pattern table.
 */
Pat_table* Song_get_pats(Song* song);


/**
 * Gets the Instruments of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Instrument table.
 */
Ins_table* Song_get_insts(Song* song);


/**
 * Gets the Effects of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Effect table.
 */
Effect_table* Song_get_effects(Song* song);


/**
 * Sets the Bind of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 * \param bind   The Bind -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_set_bind(Song* song, Bind* bind);


/**
 * Gets the array of Scales of the Song.
 *
 * \param song   The Song -- must not be \c NULL.
 *
 * \return   The Scales.
 */
Scale** Song_get_scales(Song* song);


/**
 * Sets a Scale in the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 * \param scale   The Scale -- must not be \c NULL.
 */
void Song_set_scale(Song* song, int index, Scale* scale);


/**
 * Gets a Scale of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   The Scale.
 */
Scale* Song_get_scale(Song* song, int index);


/**
 * Gets an indirect reference to the active Scale of the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 *
 * \return   The reference.
 */
Scale*** Song_get_active_scale(Song* song);


/**
 * Creates a new Scale for the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_create_scale(Song* song, int index);


/**
 * Removes a Scale from the Song.
 *
 * \param song    The Song -- must not be \c NULL.
 * \param index   The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *                If the Scale doesn't exist, nothing will be done.
 */
void Song_remove_scale(Song* song, int index);


/**
 * Destroys an existing Song.
 *
 * \param song   The Song, or \c NULL.
 */
void del_Song(Song* song);


#endif // K_SONG_H


