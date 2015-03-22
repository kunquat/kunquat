

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_AU_INTERFACE_H
#define K_AU_INTERFACE_H


#include <devices/Device.h>


typedef struct Au_interface
{
    Device parent;
} Au_interface;


/**
 * Create a new Audio unit interface.
 *
 * \return   The new Audio unit interface if successful, or \c NULL if memory
 *           allocation failed.
 */
Au_interface* new_Au_interface(void);


/**
 * Destroy an existing Audio unit interface.
 *
 * \param iface   The Audio unit interface, or \c NULL.
 */
void del_Au_interface(Au_interface* iface);


#endif // K_AU_INTERFACE_H


