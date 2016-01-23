

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_VOLUME_H
#define K_PROC_VOLUME_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_volume
{
    Device_impl parent;
    double volume;
} Proc_volume;


/**
 * Create a new volume processor.
 *
 * \return   The new volume processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_volume(void);


#endif // K_PROC_VOLUME_H


