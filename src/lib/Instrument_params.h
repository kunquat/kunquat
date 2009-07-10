

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

#include <kunquat/frame.h>
#include <Envelope.h>
#include <Note_table.h>
#include <File_base.h>
#include <File_tree.h>


typedef struct Instrument_params
{
    kqt_frame** bufs;   ///< Mixing buffer used (same as either \a pbuf or \a gbuf).
    kqt_frame** pbufs;  ///< Private mixing buffers (required when Instrument-level effects are used).
    kqt_frame** gbufs;  ///< Global mixing buffers.
    kqt_frame** vbufs;  ///< Voice buffers.
    int buf_count;    ///< Number of mixing buffers.
    uint32_t buf_len; ///< Mixing buffer length.
    
    Note_table** notes; ///< An indirect reference to the current Note table used.

    double pedal; ///< Pedal setting (0 = fully released, 1.0 = fully depressed).

    double force; ///< Force.

    bool force_volume_env_enabled; ///< Force-volume envelope toggle.
    Envelope* force_volume_env;    ///< Force-volume envelope.

    bool force_filter_env_enabled; ///< Force-filter envelope toggle.
    Envelope* force_filter_env;    ///< Force-filter envelope.

    bool force_pitch_env_enabled;  ///< Force-pitch envelope toggle.
    Envelope* force_pitch_env;     ///< Force-pitch envelope.
    double force_pitch_env_scale;  ///< Force-pitch envelope scale factor.

    double volume; ///< Instrument volume.

    bool volume_env_enabled;  ///< Volume envelope toggle.
    bool volume_env_carry;    ///< Volume envelope carry.
    Envelope* volume_env;     ///< Volume envelope.
    double volume_env_scale;  ///< Volume envelope scale factor (frequency -> speed).
    double volume_env_center; ///< Volume envelope scale center frequency.

    bool volume_off_env_enabled;  ///< Note Off volume envelope toggle.
    Envelope* volume_off_env;     ///< Note Off volume envelope.
    double volume_off_env_factor; ///< Note Off volume envelope scale factor (frequency -> speed).
    double volume_off_env_center; ///< Note Off volume envelope scale center frequency.

    bool panning_enabled;       ///< Default panning toggle.
    double panning;             ///< Default panning.
    bool pitch_pan_env_enabled; ///< Pitch-panning envelope toggle.
    Envelope* pitch_pan_env;    ///< Pitch-panning envelope.

    bool filter_env_enabled;    ///< Filter envelope toggle.
    Envelope* filter_env;       ///< Filter envelope.
    double filter_env_scale;    ///< Filter envelope scale factor (frequency -> speed).
    double filter_env_center;   ///< Filter envelope scale center frequency.

    bool filter_off_env_enabled;  ///< Note Off filter envelope toggle.
    Envelope* filter_off_env;     ///< Note Off filter envelope.
    double filter_off_env_scale;  ///< Note Off filter envelope scale factor (frequency -> speed).
    double filter_off_env_center; ///< Note Off filter envelope scale center frequency.
} Instrument_params;


/**
 * Initialises the Instrument parameters.
 *
 * \param ip          The Instrument parameters -- must not be \c NULL.
 * \param bufs        The global mixing buffers -- must not be \c NULL and must
 *                    contain at least two buffers.
 * \param vbufs       The Voice mixing buffers -- must not be \c NULL and must
 *                    contain at least two buffers.
 * \param buf_count   The number of buffers -- must be > \c 0.
 * \param buf_len     The length of the buffers -- must be > \c 0.
 * \param notes       An indirect reference to the Note table -- must not be
 *                    \c NULL.
 *
 * \return   The parameter \a ip if successful, or \c NULL if memory
 *           allocation failed.
 */
Instrument_params* Instrument_params_init(Instrument_params* ip,
        kqt_frame** bufs,
        kqt_frame** vbufs,
        int buf_count,
        uint32_t buf_len,
        Note_table** notes);


/**
 * Reads Instrument parameters from a File tree.
 *
 * \param ip      The Instrument parameters -- must not be \c NULL.
 * \param tree    The File tree -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Instrument_params_read(Instrument_params* ip, File_tree* tree, Read_state* state);


/**
 * Uninitialises the Instrument parameters.
 *
 * \param ip   The Instrument parameters -- must not be \c NULL.
 */
void Instrument_params_uninit(Instrument_params* ip);


#endif // K_INSTRUMENT_PARAMS_H


