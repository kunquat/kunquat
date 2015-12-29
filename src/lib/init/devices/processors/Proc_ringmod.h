

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_RINGMOD_H
#define K_PROC_RINGMOD_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_ringmod
{
    Device_impl parent;
} Proc_ringmod;


/**
 * Create a new Ring modulator Processor.
 *
 * \return   The new Ring modulator Processor if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_impl* new_Proc_ringmod(Processor* proc);


#endif // K_PROC_RINGMOD_H


