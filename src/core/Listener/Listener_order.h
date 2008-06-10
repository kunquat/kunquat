

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


#ifndef K_LISTENER_ORDER_H
#define K_LISTENER_ORDER_H


#include "Listener.h"


/**
 * The Order part of the Listener will respond to most methods with the
 * response method <host_path>/order_info which contains the following
 * arguments:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li Zero or more of the following (starting from order 0):
 * \li \li \c i   The Pattern number.
 */


/**
 * Gets all the order lists of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 */
int Listener_get_orders(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Sets an order of the given Song.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li \c i   The order number.
 * \li \c i   The Pattern number.
 */
int Listener_set_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Inserts an order slot. The Pattern number will be ORDER_NONE.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li \c i   The order number.
 */
int Listener_ins_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Removes an order slot.
 *
 * The following OSC arguments are expected:
 *
 * \li \c i   The Song ID.
 * \li \c i   The subsong number.
 * \li \c i   The order number.
 */
int Listener_del_order(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_ORDER_H


