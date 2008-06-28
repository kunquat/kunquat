

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


#include "Listener.h"


/**
 * The Instrument part of the Listener will respond to many methods with the
 * response method <host_path>/ins_info which contains the following arguments
 * if the host call has succeeded:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The type of the Instrument.
 * \li \c s   The name of the Instrument.
 * \li        Zero or more additional arguments dependent on the Instrument type.
 *
 * The None, Debug and Sine types have no additional arguments.
 */

/**
 * Type information of an Instrument is sent via the method
 * <host_path>/ins_type_desc which contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li        The following arguments zero or more times:
 * \li \li \c s   The category of the field.
 * \li \li \c s   The name of the field.
 * \li \li \c s   The type of the field.
 * \li \li \c s   The constraints of the field.
 *
 * Possible field types are:
 *
 * \li "p"   Path.
 */

/**
 * A type-specific field of an Instrument is sent via the method
 * <host_path>/ins_type_field which contains the following arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the field.
 * \li        Other fields dependent on the type.
 */


/**
 * Gets information on all the Instruments of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 *
 * The response consists of one ins_info method call for each existing
 * Instrument.
 */
Listener_callback Listener_get_insts;


/**
 * Creates a new Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255). An Instrument with the same
 *            number will be removed if one exists.
 * \li \c i   The type of the new Instrument.
 */
Listener_callback Listener_new_ins;


/**
 * Sets the name of the Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c s   The name.
 */
Listener_callback Listener_ins_set_name;


/**
 * Gets the type field description of the Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 *
 * The response method is <host_path>/ins_type_desc.
 */
Listener_callback Listener_ins_get_type_desc;


/**
 * Gets a type-specific field value of the Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the field (>= 0).
 *
 * The response method is <host_path>/ins_type_field.
 */
Listener_callback Listener_ins_get_type_field;


/**
 * Gets a type-specific field value of the Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 * \li \c i   The index of the field.
 * \li        One or more fields dependent on the type.
 *
 * The response method is <host_path>/ins_type_field.
 */
Listener_callback Listener_ins_set_type_field;


/**
 * Removes an Instrument.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The Instrument number (1..255).
 */
Listener_callback Listener_del_ins;


#endif // K_LISTENER_INS_H


