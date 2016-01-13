

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_PITCH_H
#define K_PROC_PITCH_H


#include <decl.h>
#include <init/devices/Device_impl.h>


typedef struct Proc_pitch
{
    Device_impl parent;
} Proc_pitch;


/**
 * Create a new pitch processor.
 *
 * \return   The new pitch processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_pitch(void);


#endif // K_PROC_PITCH_H


