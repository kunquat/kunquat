

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


#ifndef K_LISTENER_DRIVER_H
#define K_LISTENER_DRIVER_H


#include "Listener.h"


/**
 * Gets a list of available audio drivers.
 *
 * A response message contains driver names as separate strings. The drivers
 * can be referred to as 0-based indexes based on the order they appear on the
 * list.
 */
Listener_callback Listener_get_drivers;


/**
 * Gets the ID of active audio driver.
 *
 * A response message will contain the ID of the active audio driver and the
 * current mixing frequency.
 */
Listener_callback Listener_active_driver;


/**
 * Initialises a sound driver.
 *
 * The following OSC parameters are expected:
 *
 * \li \c i   The 0-based index of the driver in the list of drivers returned
 *            by get_drivers.
 *
 * If initialisation succeeded, the response message contains the mixing
 * frequency. Otherwise, an error message will be sent.
 */
Listener_callback Listener_driver_init;


/**
 * Uninitialises the current sound driver.
 *
 * A response message is a notification "Closed driver #"
 */
Listener_callback Listener_driver_close;


#endif // K_LISTENER_DRIVER_H


