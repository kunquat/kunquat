

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MASTER_PARAMS_H
#define K_MASTER_PARAMS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <Decl.h>
#include <General_state.h>
#include <player/Active_jumps.h>
#include <player/Env_state.h>
#include <player/Jump_cache.h>
#include <player/Position.h>
#include <player/Slider.h>


typedef enum
{
    PLAYBACK_STOPPED = 0,
    PLAYBACK_PATTERN,
    PLAYBACK_SONG,
    PLAYBACK_MODULE,
    PLAYBACK_COUNT
} Playback_state;


typedef struct Master_params
{
    General_state parent;

    // Playback session identifier (essentially a reset counter)
    uint32_t playback_id;

    // Start params
    Position start_pos;

    // Read/write state for trigger events
    Playback_state playback_state;
    bool is_infinite;

    Position cur_pos;
    int cur_ch;
    int cur_trigger;

    Tstamp delay_left;

    bool   tempo_settings_changed;
    double tempo;
    int    tempo_slide;
    Tstamp tempo_slide_length;
    double tempo_slide_target;
    Tstamp tempo_slide_left;
    Tstamp tempo_slide_slice_left;
    double tempo_slide_update;

    double volume;
    Slider volume_slider;

    bool          do_jump;
    int16_t       jump_counter;
    Pat_inst_ref  jump_target_piref;
    Tstamp        jump_target_row;
    Active_jumps* active_jumps;
    Jump_cache*   jump_cache;

    // Statistics
    int16_t active_voices;
} Master_params;


/**
 * Pre-initialises the Master params.
 *
 * This function must be called before Maste_params_init on given parameters.
 *
 * \param params   The Master params -- must not be \c NULL.
 *
 * \return   The parameter \a params.
 */
Master_params* Master_params_preinit(Master_params* params);


/**
 * Initialises the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 * \param module   The Module -- must not be \c NULL.
 * \param estate   The Environment state -- must not be \c NULL.
 *
 * \return   The parameter \a params if successful, or \c NULL if memory
 *           allocation failed.
 */
Master_params* Master_params_init(
        Master_params* params,
        const Module* module,
        Env_state* estate);


/**
 * Sets the starting tempo.
 *
 * This function is only needed before reading patterns for the first time.
 *
 * \param params   The Master params -- must not be \c NULL.
 */
void Master_params_set_starting_tempo(Master_params* params);


/**
 * Resets the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 * \param module   The Module -- must not be \c NULL.
 */
void Master_params_reset(Master_params* params);


/**
 * Deinitialises the Master params.
 *
 * \param params   The Master params -- must not be \c NULL.
 */
void Master_params_deinit(Master_params* params);


#endif // K_MASTER_PARAMS_H


