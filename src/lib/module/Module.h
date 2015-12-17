

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


#ifndef K_MODULE_H
#define K_MODULE_H


#include <stdint.h>

#include <Connections.h>
#include <Decl.h>
#include <devices/Device.h>
#include <kunquat/limits.h>
#include <module/Bind.h>
#include <module/Environment.h>
#include <module/Input_map.h>
#include <module/Au_table.h>
#include <module/Scale.h>
#include <module/sheet/Channel_defaults_list.h>
#include <module/sheet/Order_list.h>
#include <module/sheet/Pat_table.h>
#include <module/sheet/Song_table.h>
#include <module/sheet/Track_list.h>
#include <string/Streader.h>


struct Module
{
    Device parent;
    uint64_t random_seed;               ///< The master random seed of the Module.
    Song_table* songs;                  ///< The Songs.
    bool album_is_existent;             ///< Album existence status.
    Track_list* track_list;             ///< Track list.
    Order_list* order_lists[KQT_SONGS_MAX]; ///< Order lists.
    Channel_defaults_list* ch_defs;     ///< Channel defaults.
    Pat_table* pats;                    ///< The Patterns.
    Input_map* au_map;                  ///< Audio unit input map.
    Bit_array* au_controls;             ///< Audio unit control existence info.
    Au_table* au_table;                 ///< The Audio units.
    Connections* connections;           ///< Device connections.
    Scale* scales[KQT_SCALES_MAX];      ///< The Scales.
    double mix_vol_dB;                  ///< Mixing volume in dB.
    double mix_vol;                     ///< Mixing volume.
    Environment* env;                   ///< Environment variables.
    Bind* bind;
};


#define MODULE_DEFAULT_BUF_COUNT 2
#define MODULE_DEFAULT_MIX_VOL -8
#define MODULE_DEFAULT_INIT_SONG 0


/**
 * Create a new Module.
 *
 * The caller shall eventually call del_Module() to destroy the Module returned.
 *
 * \return   The new Module if successful, or \c NULL if memory allocation
 *           failed.
 */
Module* new_Module(void);


/**
 * Parse the composition header of a Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_parse_composition(Module* module, Streader* sr);


/**
 * Parse the random seed of the composition.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_parse_random_seed(Module* module, Streader* sr);


/**
 * Get the track list of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The track list if the Module has an album, otherwise \c NULL.
 */
const Track_list* Module_get_track_list(const Module* module);


/**
 * Get an order list of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param song     The song number -- must be >= \c 0 and < \c KQT_SONGS_MAX.
 *
 * \return   The order list if \a song exists, otherwise \c NULL.
 */
const Order_list* Module_get_order_list(const Module* module, int16_t song);


/**
 * Set the list of channel defaults.
 *
 * \param module    The Module -- must not be \c NULL.
 * \param ch_defs   The Channel defaults list, or \c NULL.
 */
void Module_set_ch_defaults_list(Module* module, Channel_defaults_list* ch_defs);


/**
 * Get the list of channel defaults in the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The channel defaults list if one exists, otherwise \c NULL.
 */
const Channel_defaults_list* Module_get_ch_defaults_list(const Module* module);


/**
 * Get a pattern of the Module.
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
 * Find the location of a pattern instance inside the Module.
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
 * Set the mixing volume of the Module.
 *
 * \param module    The Module -- must not be \c NULL.
 * \param mix_vol   The volume -- must be finite or -INFINITY.
 */
void Module_set_mix_vol(Module* module, double mix_vol);


/**
 * Get the mixing volume of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The mixing volume.
 */
double Module_get_mix_vol(Module* module);


/**
 * Get the Song table of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Song table.
 */
Song_table* Module_get_songs(const Module* module);


/**
 * Get the Patterns of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Pattern table.
 */
Pat_table* Module_get_pats(Module* module);


/**
 * Get the Audio unit corresponding to an input index.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Audio unit if one exists, otherwise \c NULL.
 */
Audio_unit* Module_get_au_from_input(const Module* module, int32_t input);


/**
 * Set the existence status of a control.
 *
 * \param module     The Module -- must not be \c NULL.
 * \param control    The control index -- must be >= \c 0 and
 *                   < \c KQT_CONTROLS_MAX.
 * \param existent   \c true if the control exists, otherwise \c false.
 */
void Module_set_control(Module* module, int control, bool existent);


/**
 * Get the existence status of a control.
 *
 * \param module    The Module -- must not be \c NULL.
 * \param control   The control index -- must be >= \c 0 and
 *                  < \c KQT_CONTROLS_MAX.
 *
 * \return   \c true if the control exists, otherwise \c false.
 */
bool Module_get_control(const Module* module, int control);


/**
 * Set the Audio unit input map of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the input data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_set_au_map(Module* module, Streader* sr);


/**
 * Get the Audio unit input map of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Audio unit input map.
 */
Input_map* Module_get_au_map(const Module* module);


/**
 * Get the Audio units of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Audio unit table.
 */
Au_table* Module_get_au_table(const Module* module);


/**
 * Set the Bind of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param bind     The Bind -- must not be \c NULL.
 */
void Module_set_bind(Module* module, Bind* bind);


/**
 * Set a Scale in the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 * \param scale    The Scale -- must not be \c NULL.
 */
void Module_set_scale(Module* module, int index, Scale* scale);


/**
 * Get a Scale of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   The Scale.
 */
Scale* Module_get_scale(Module* module, int index);


/**
 * Create a new Scale for the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Module_create_scale(Module* module, int index);


/**
 * Remove a Scale from the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The Scale index -- must be >= 0 and < KQT_SCALES_MAX.
 *                 If the Scale doesn't exist, nothing will be done.
 */
void Module_remove_scale(Module* module, int index);


/**
 * Destroy an existing Module.
 *
 * \param module   The Module, or \c NULL.
 */
void del_Module(Module* module);


#endif // K_MODULE_H


