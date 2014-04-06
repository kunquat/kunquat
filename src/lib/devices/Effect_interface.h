

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EFFECT_INTERFACE_H
#define K_EFFECT_INTERFACE_H


#include <devices/Device.h>


typedef struct Effect_interface
{
    Device parent;
} Effect_interface;


/**
 * Create a new Effect interface.
 *
 * \return   The new Effect interface if successful, or \c NULL if memory
 *           allocation failed.
 */
Effect_interface* new_Effect_interface(void);


/**
 * Destroy an existing Effect interface.
 *
 * \param ei   The Effect interface, or \c NULL.
 */
void del_Effect_interface(Effect_interface* ei);


#endif // K_EFFECT_INTERFACE_H


