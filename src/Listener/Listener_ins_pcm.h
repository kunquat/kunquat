

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_LISTENER_INS_PCM_H
#define K_LISTENER_INS_PCM_H


#include "Listener.h"


/**
 * The response call <host_path>/ins_info contains the following extra
 * arguments for PCM instruments:
 *
 * For each existing Sample:
 *
 * \li \c i   The Sample number.
 * \li \c s   The path of the Sample.
 * \li \c d   The middle frequency of the Sample.
 *
 * These are followed by the string "__styles". Then, for each style defined:
 *
 * \li \c i   The number of the style.
 * \li \c s   The name of the style.
 *
 * These are followed by the string "__maps". Then, for each mapping defined:
 *
 * \li \c i   The sound source to which this mapping is applied.
 * \li \c i   The index of the style to which this mapping is applied.
 * \li \c i   The number of strength levels.
 * \li        For each strength level:
 * \li \li \c d   The lower threshold strength for the level.
 * \li \li \c i   The number of frequency levels.
 * \li \li        For each frequency level:
 * \li \li \li \c d   The lower threshold frequency for the level.
 * \li \li \li \c i   The number of samples for this frequency/strength level.
 * \li \li \li        For each sample:
 * \li \li \li \li \c i   The Sample number.
 * \li \li \li \li \c d   The frequency scale factor for this sample.
 * \li \li \li \li \c d   The volume scale factor for this sample.
 */


/**
 * Adds parameters specific to the PCM Instrument type into an outgoing
 * ins_info call.
 *
 * \param lr    The Listener -- must not be \c NULL.
 * \param m     The OSC message -- must not be \c NULL.
 * \param ins   The Instrument -- must not be \c NULL and must be a PCM
 *              Instrument.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool ins_info_pcm(Listener* lr, lo_message m, Instrument* ins);


/**
 * Gets detailed information about a Sample in a PCM Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the Sample.
 */
Listener_callback Listener_ins_pcm_get_sample_info;


/**
 * Loads a Sample into a PCM Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the Sample.
 * \li \c s   The path of the Sample.
 */
Listener_callback Listener_ins_pcm_load_sample;


/**
 * Sets the middle frequency of a Sample.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the Sample.
 * \li \c d   The middle frequency.
 */
Listener_callback Listener_ins_pcm_sample_set_mid_freq;


/**
 * Removes a Sample from a PCM Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the Sample.
 */
Listener_callback Listener_ins_pcm_remove_sample;


/**
 * Sets a Sample mapping of a PCM Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The sound source.
 * \li \c i   The index of the style.
 * \li \c i   The index of the strength level.
 * \li \c d   The lower-bound frequency.
 * \li \c i   The index of the entry among random-choice Samples.
 * \li \c i   The Sample table index.
 * \li \c d   The frequency scale factor.
 * \li \c d   The volume scale factor.
 */
Listener_callback Listener_ins_pcm_set_mapping;


/**
 * Removes a Sample mapping from a PCM Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The sound source.
 * \li \c i   The index of the style.
 * \li \c i   The index of the strength level.
 * \li \c d   The lower-bound frequency.
 * \li \c i   The index of the entry among random-choice Samples.
 */
Listener_callback Listener_ins_pcm_del_mapping;


#endif // K_LISTENER_INS_PCM_H


