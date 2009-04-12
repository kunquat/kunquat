

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_INSTRUMENT_PARAMS_H
#define K_INSTRUMENT_PARAMS_H


#include <stdint.h>
#include <stdbool.h>

#include <frame_t.h>
#include <Envelope.h>


typedef struct Instrument_params
{
    /// Mixing buffer used (same as either \a pbuf or \a gbuf).
    frame_t** bufs;
    /// Private mixing buffer (required when Instrument-level effects are used).
    frame_t** pbufs;
    /// Global mixing buffer.
    frame_t** gbufs;
    /// Mixing buffer length.
    uint32_t buf_len;

    /// Pedal setting (0 = fully released, 1.0 = fully depressed).
    double pedal;

    /// Force.
    double force;

    /// Force-volume envelope toggle.
    bool force_volume_env_enabled;
    /// Force-volume envelope.
    Envelope* force_volume_env;

    /// Force-filter envelope toggle.
    bool force_filter_env_enabled;
    /// Force-filter envelope.
    Envelope* force_filter_env;

    /// Force-pitch envelope toggle.
    bool force_pitch_env_enabled;
    /// Force-pitch envelope.
    Envelope* force_pitch_env;
    /// Force-pitch envelope scale factor.
    double force_pitch_env_scale;

    /// Instrument volume.
    double volume;

    /// Volume envelope toggle.
    bool volume_env_enabled;
    /// Volume envelope carry.
    bool volume_env_carry;
    /// Volume envelope.
    Envelope* volume_env;
    /// Volume envelope scale factor (frequency -> speed).
    double volume_env_scale;
    /// Volume envelope scale center frequency.
    double volume_env_center;

    /// Note Off volume envelope toggle.
    bool volume_off_env_enabled;
    /// Note Off volume envelope.
    Envelope* volume_off_env;
    /// Note Off volume envelope scale factor (frequency -> speed).
    double volume_off_env_scale;
    /// Note Off volume envelope scale center frequency.
    double volume_off_env_center;

    /// Default panning toggle.
    bool panning_enabled;
    /// Default panning.
    double panning;
    /// Pitch-panning envelope toggle.
    bool pitch_pan_env_enabled;
    /// Pitch-panning envelope.
    Envelope* pitch_pan_env;

    /// Filter envelope toggle.
    bool filter_env_enabled;
    /// Filter envelope.
    Envelope* filter_env;
    /// Filter envelope scale factor (frequency -> speed).
    double filter_env_scale;
    /// Filter envelope scale center frequency.
    double filter_env_center;

    /// Note Off filter envelope toggle.
    bool filter_off_env_enabled;
    /// Note Off filter envelope.
    Envelope* filter_off_env;
    /// Note Off filter envelope scale factor (frequency -> speed).
    double filter_off_env_scale;
    /// Note Off filter envelope scale center frequency.
    double filter_off_env_center;
} Instrument_params;


/**
 * Initialises the Instrument parameters.
 *
 * \param ip        The Instrument parameters -- must not be \c NULL.
 * \param bufs      The global mixing buffers -- must not be \c NULL and must
 *                  contain at least two buffers.
 * \param buf_len   The length of the buffers -- must be > \c 0.
 *
 * \return   The parameter \a ip if successful, or \c NULL if memory
 *           allocation failed.
 */
Instrument_params* Instrument_params_init(Instrument_params* ip,
        frame_t** bufs,
        uint32_t buf_len);


/**
 * Uninitialises the Instrument parameters.
 *
 * \param ip   The Instrument parameters -- must not be \c NULL.
 */
void Instrument_params_uninit(Instrument_params* ip);


#endif // K_INSTRUMENT_PARAMS_H


