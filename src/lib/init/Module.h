

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


#ifndef KQT_MODULE_H
#define KQT_MODULE_H


#include <decl.h>
#include <init/Bind.h>
#include <init/Connections.h>
#include <init/devices/Device.h>
#include <init/Environment.h>
#include <init/Input_map.h>
#include <init/Au_table.h>
#include <init/sheet/Channel_defaults_list.h>
#include <init/sheet/Order_list.h>
#include <init/sheet/Pat_table.h>
#include <init/sheet/Song_table.h>
#include <init/sheet/Track_list.h>
#include <init/Tuning_table.h>
#include <kunquat/limits.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


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
    Tuning_table* tuning_tables[KQT_TUNING_TABLES_MAX];
    bool is_dc_blocker_enabled;         ///< Block dc in the final mix.
    double mix_vol_dB;                  ///< Mixing volume in dB.
    double mix_vol;                     ///< Mixing volume.
    double force_shift;                 ///< Force shift.
    Environment* env;                   ///< Environment variables.
    Bind* bind;
};


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
 * Read the dc blocker status of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_read_dc_blocker_enabled(Module* module, Streader* sr);


/**
 * Read the mixing volume of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_read_mixing_volume(Module* module, Streader* sr);


/**
 * Read the force shift of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param sr       The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Module_read_force_shift(Module* module, Streader* sr);


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
const Order_list* Module_get_order_list(const Module* module, int song);


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
const Pattern* Module_get_pattern(const Module* module, const Pat_inst_ref* piref);


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
        const Module* module, const Pat_inst_ref* piref, int* track, int* system);


/**
 * Get the Connections of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The Connections.
 */
const Connections* Module_get_connections(const Module* module);


/**
 * Set a Tuning table in the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The table index -- must be >= 0 and < KQT_TUNING_TABLES_MAX.
 * \param tt       The Tuning table, or \c NULL.
 */
void Module_set_tuning_table(Module* module, int index, Tuning_table* tt);


/**
 * Get a Tuning table of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param index    The table index -- must be >= 0 and < KQT_TUNING_TABLES_MAX.
 *
 * \return   The Tuning table, or \c NULL if there is no table at \a index.
 */
const Tuning_table* Module_get_tuning_table(const Module* module, int index);


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
 * Destroy an existing Module.
 *
 * \param module   The Module, or \c NULL.
 */
void del_Module(Module* module);


#endif // KQT_MODULE_H


