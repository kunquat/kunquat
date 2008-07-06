

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


#endif // K_LISTENER_INS_PCM_H


