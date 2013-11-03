

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
#include <Effect_table.h>
#include <File_base.h>
#include <frame.h>
#include <Input_map.h>
#include <Ins_table.h>
#include <kunquat/limits.h>
#include <Order_list.h>
#include <Pat_table.h>
#include <Random.h>
#include <Scale.h>
#include <Song_table.h>
#include <Track_list.h>


struct Module
{
    Device parent;
    uint64_t random_seed;               ///< The master random seed of the Module.
    Random* random;                     ///< The source for random data in the Module.
    Song_table* songs;                  ///< The Songs.
    bool album_is_existent;             ///< Album existence status.
    Track_list* track_list;             ///< Track list.
    Order_list* order_lists[KQT_SONGS_MAX]; ///< Order lists.
    Pat_table* pats;                    ///< The Patterns.
    Input_map* ins_map;                 ///< Instrument input map.
    Ins_table* insts;                   ///< The Instruments.
    Effect_table* effects;              ///< The global Effects.
    Connections* connections;           ///< Device connections.
    Scale* scales[KQT_SCALES_MAX];      ///< The Scales.
    double mix_vol_dB;                  ///< Mixing volume in dB.
    double mix_vol;                     ///< Mixing volume.
    Environment* env;                   ///< Environment variables.
    Bind* bind;
};


#define MODULE_DEFAULT_BUF_COUNT (2)
#define MODULE_DEFAULT_MIX_VOL (-8)
#define MODULE_DEFAULT_INIT_SONG (0)


/**
 * Creates a new Module.
 *
 * The caller shall eventually call del_Module() to destroy the Module returned.
 *
 * \return   The new Module if successful, or \c NULL if memory allocation
 *           failed.
 */
Module* new_Module();


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
 * Gets the Song table of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Song table.
 */
Song_table* Module_get_songs(const Module* module);


/**
 * Gets the Patterns of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Pattern table.
 */
Pat_table* Module_get_pats(Module* module);


/**
 * Gets the Instrument corresponding to an input index.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Instrument.
 */
Instrument* Module_get_ins_from_input(const Module* module, int32_t input);


/**
 * Sets the Instrument input map of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the input data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_set_ins_map(Module* module, Streader* sr);


/**
 * Gets the Instrument input map of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Instrument input map.
 */
Input_map* Module_get_ins_map(const Module* module);


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
 */
void Module_set_bind(Module* module, Bind* bind);


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


