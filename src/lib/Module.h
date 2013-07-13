

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MODULE_H
#define K_MODULE_H


#include <stdint.h>

#include <Bind.h>
#include <Connections.h>
#include <Decl.h>
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
#include <Track_list.h>
#include <Playdata.h>
#include <File_base.h>
#include <Event_handler.h>


struct Module
{
    Device parent;
    uint64_t random_seed;               ///< The master random seed of the Module.
    Random* random;                     ///< The source for random data in the Module.
    Subsong_table* subsongs;            ///< The Subsongs.
    bool album_is_existent;             ///< Album existence status.
    Track_list* track_list;             ///< Track list.
    Order_list* order_lists[KQT_SONGS_MAX]; ///< Order lists.
    Pat_table* pats;                    ///< The Patterns.
    Ins_table* insts;                   ///< The Instruments.
    Effect_table* effects;              ///< The global Effects.
    Connections* connections;           ///< Device connections.
    Scale* scales[KQT_SCALES_MAX];      ///< The Scales.
    double mix_vol_dB;                  ///< Mixing volume in dB.
    double mix_vol;                     ///< Mixing volume.
//    uint16_t init_subsong;              ///< Initial subsong number.
    Playdata* play_state;               ///< Playback state.
    Playdata* skip_state;               ///< Skip state (used for length calculation).
    Environment* env;                   ///< Environment variables.
    Bind* bind;
};


#define SONG_DEFAULT_BUF_COUNT (2)
#define SONG_DEFAULT_MIX_VOL (-8)
#define SONG_DEFAULT_INIT_SUBSONG (0)


/**
 * Creates a new Module.
 *
 * The caller shall eventually call del_Module() to destroy the Module returned.
 *
 * \param buf_size   Size of the mixing buffer -- must be > \c 0 and
 *                   <= KQT_BUFFER_SIZE_MAX.
 *
 * \see del_Module()
 *
 * \return   The new Module if successful, or \c NULL if memory allocation
 *           failed.
 */
Module* new_Module(uint32_t buf_size);


/**
 * Parses the composition header of a Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param str      The textual description -- must not be \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_parse_composition(Module* module, char* str, Read_state* state);


/**
 * Parses the random seed of the composition.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param str      The textual description -- must not be \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_parse_random_seed(Module* module, char* str, Read_state* state);


/**
 * Gets the track list of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The track list if the Module has an album, otherwise \c NULL.
 */
const Track_list* Module_get_track_list(const Module* module);


/**
 * Gets an order list of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param song     The song number -- must be >= \c 0 and < \c KQT_SONGS_MAX.
 *
 * \return   The order list if \a song exists, otherwise \c NULL.
 */
const Order_list* Module_get_order_list(const Module* module, int16_t song);


/**
 * Gets a pattern of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param piref    The pattern instance -- must be valid.
 *
 * \return   The pattern if one exists, otherwise \c NULL.
 */
const Pattern* Module_get_pattern(
        const Module* module,
        const Pat_inst_ref* piref);


/**
 * Finds the location of a pattern instance inside the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param piref    The pattern instance -- must be valid.
 * \param track    Address where the track number will be stored -- must not
 *                 be \c NULL.
 * \param system   Address where the system number will be stored -- must not
 *                 be \c NULL.
 *
 * \return   \c true if \a piref was found, otherwise \c false.
 */
bool Module_find_pattern_location(
        const Module* module,
        const Pat_inst_ref* piref,
        int16_t* track,
        int16_t* system);


/*
 * FIXME: Old interface below, clean up
 */


/**
 * Sets the mixing volume of the Module.
 *
 * \param module    The Module -- must not be \c NULL.
 * \param mix_vol   The volume -- must be finite or -INFINITY.
 */
void Module_set_mix_vol(Module* module, double mix_vol);


/**
 * Gets the mixing volume of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The mixing volume.
 */
double Module_get_mix_vol(Module* module);


/**
 * Gets the Subsong table from the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Subsong table.
 */
Subsong_table* Module_get_subsongs(const Module* module);


/**
 * Gets the Patterns of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Pattern table.
 */
Pat_table* Module_get_pats(Module* module);


/**
 * Gets the Instruments of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Instrument table.
 */
Ins_table* Module_get_insts(const Module* module);


/**
 * Gets the Effects of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Effect table.
 */
Effect_table* Module_get_effects(const Module* module);


/**
 * Sets the Bind of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param bind     The Bind -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Module_set_bind(Module* module, Bind* bind);


/**
 * Gets the array of Scales of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Scales.
 */
Scale** Module_get_scales(Module* module);


/**
 * Sets a Scale in the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 * \param scale    The Scale -- must not be \c NULL.
 */
void Module_set_scale(Module* module, int index, Scale* scale);


/**
 * Gets a Scale of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   The Scale.
 */
Scale* Module_get_scale(Module* module, int index);


/**
 * Gets an indirect reference to the active Scale of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The reference.
 */
Scale*** Module_get_active_scale(Module* module);


/**
 * Creates a new Scale for the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Module_create_scale(Module* module, int index);


/**
 * Removes a Scale from the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *                 If the Scale doesn't exist, nothing will be done.
 */
void Module_remove_scale(Module* module, int index);


/**
 * Destroys an existing Module.
 *
 * \param module   The Module, or \c NULL.
 */
void del_Module(Module* module);


#endif // K_MODULE_H


