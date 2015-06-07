

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


#ifndef K_AU_PARAMS_H
#define K_AU_PARAMS_H


#include <stdint.h>
#include <stdbool.h>

#include <devices/param_types/Envelope.h>
#include <frame.h>
#include <module/Scale.h>
#include <string/Streader.h>


typedef struct Au_params
{
    uint32_t device_id;

    // Basic force settings
    double global_force;
    double force;
    double force_variation;

    // Force -> param envelopes
    bool force_volume_env_enabled;
    Envelope* force_volume_env;

    bool env_force_filter_enabled;
    Envelope* env_force_filter;

    bool force_pitch_env_enabled;
    Envelope* force_pitch_env;
    double force_pitch_env_scale;

    double volume; ///< Audio unit volume.

    // Force envelopes
    bool env_force_enabled;
    bool env_force_loop_enabled;
    bool env_force_carry;
    Envelope* env_force;
    double env_force_scale_amount;
    double env_force_center;

    bool env_force_rel_enabled;
    Envelope* env_force_rel;
    double env_force_rel_scale_amount;
    double env_force_rel_center;

    // Panning settings
    bool panning_enabled;
    double panning;
    bool env_pitch_pan_enabled;
    Envelope* env_pitch_pan;

    // Lowpass filter settings
    double global_lowpass;
    double default_lowpass;
    double default_resonance;
    double pitch_lowpass_scale;
    bool env_filter_enabled;
    bool env_filter_loop_enabled;
    Envelope* env_filter;
    double env_filter_scale_amount;
    double env_filter_scale_center;

    bool env_filter_rel_enabled;
    Envelope* env_filter_rel;
    double env_filter_rel_scale_amount;
    double env_filter_rel_scale_center;
} Au_params;


/**
 * Initialise the Audio unit parameters.
 *
 * \param aup         The Audio unit parameters -- must not be \c NULL.
 * \param device_id   The audio unit device ID -- must be > \c 0.
 *
 * \return   The parameter \a aup if successful, or \c NULL if memory
 *           allocation failed.
 */
Au_params* Au_params_init(Au_params* aup, uint32_t device_id);


/**
 * Parse an Audio unit parameter file.
 *
 * \param aup   The Audio unit parameters -- must not be \c NULL.
 * \param sr    The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Au_params_parse_env_force_rel(Au_params* aup, Streader* sr);


bool Au_params_parse_env_force(Au_params* aup, Streader* sr);


bool Au_params_parse_env_force_filter(Au_params* aup, Streader* sr);


bool Au_params_parse_env_pitch_pan(Au_params* aup, Streader* sr);


bool Au_params_parse_env_filter(Au_params* aup, Streader* sr);


bool Au_params_parse_env_filter_rel(Au_params* aup, Streader* sr);


/**
 * Deinitialise the Audio unit parameters.
 *
 * \param aup   The Audio unit parameters, or \c NULL.
 */
void Au_params_deinit(Au_params* aup);


#endif // K_AU_PARAMS_H


