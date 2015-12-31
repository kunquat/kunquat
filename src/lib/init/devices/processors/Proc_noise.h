

/*
 * Authors: Tomi Jylhä-Ollila, Finland 2010-2015
 *          Ossi Saresoja, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_NOISE_H
#define K_PROC_NOISE_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_noise
{
    Device_impl parent;
} Proc_noise;


/**
 * Create a new Noise processor.
 *
 * \return   The new Noise processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_noise(void);


#endif // K_PROC_NOISE_H


