

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


#ifndef K_LISTENER_INS_H
#define K_LISTENER_INS_H


#include "lo/lo.h"


/**
 * The Instrument part of the Listener will respond to all methods with the
 * response method <host_path>/ins_info which contains the following arguments
 * if the host call has succeeded:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The type of the Instrument.
 * \li \c s   The name of the Instrument.
 * \li        Zero or more additional parameters dependent on the Instrument type.
 *
 * The Debug and Sine types have no additional parameters.
 */


/**
 * Creates a new Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1--255). An Instrument with the same
 *            number will be removed if one exists.
 * \li \c i   The type of the new Instrument.
 */
int Listener_new_ins(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Sets the name of the Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c s   The name.
 */
int Listener_ins_set_name(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Removes an Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 */
int Listener_del_ins(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_INS_H


