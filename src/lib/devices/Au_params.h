

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

    double global_force;           ///< Global force.
    double force;                  ///< Default force.
    double force_variation;        ///< Force variation.

    bool force_volume_env_enabled; ///< Force-volume envelope toggle.
    Envelope* force_volume_env;    ///< Force-volume envelope.

    bool env_force_filter_enabled; ///< Force-filter envelope toggle.
    Envelope* env_force_filter;    ///< Force-filter envelope.

    bool force_pitch_env_enabled;  ///< Force-pitch envelope toggle.
    Envelope* force_pitch_env;     ///< Force-pitch envelope.
    double force_pitch_env_scale;  ///< Force-pitch envelope scale factor.

    double volume; ///< Audio unit volume.

    bool env_force_enabled;  ///< Force envelope toggle.
    bool env_force_loop_enabled;
    bool env_force_carry;    ///< Force envelope carry.
    Envelope* env_force;     ///< Force envelope.
    double env_force_scale_amount; ///< Force envelope scale amount (frequency -> speed).
    double env_force_center; ///< Force envelope scale center frequency.

    bool env_force_rel_enabled;  ///< Release force envelope toggle.
    Envelope* env_force_rel;     ///< Release force envelope.
    double env_force_rel_scale_amount; ///< Release force envelope scale amount (frequency -> speed).
    double env_force_rel_center; ///< Release force envelope scale center frequency.

    bool panning_enabled;       ///< Default panning toggle.
    double panning;             ///< Default panning.
    bool env_pitch_pan_enabled; ///< Pitch-panning envelope toggle.
    Envelope* env_pitch_pan;    ///< Pitch-panning envelope.

    bool filter_env_enabled;    ///< Filter envelope toggle.
    Envelope* filter_env;       ///< Filter envelope.
    double filter_env_scale;    ///< Filter envelope scale factor (frequency -> speed).
    double filter_env_center;   ///< Filter envelope scale center frequency.

    bool filter_off_env_enabled;  ///< Note Off filter envelope toggle.
    Envelope* filter_off_env;     ///< Note Off filter envelope.
    double filter_off_env_scale;  ///< Note Off filter envelope scale factor (frequency -> speed).
    double filter_off_env_center; ///< Note Off filter envelope scale center frequency.
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


/**
 * Deinitialise the Audio unit parameters.
 *
 * \param aup   The Audio unit parameters, or \c NULL.
 */
void Au_params_deinit(Au_params* aup);


#endif // K_AU_PARAMS_H


