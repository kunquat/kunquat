

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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


#include <stdint.h>

#include <devices/Device_impl.h>
#include <devices/Processor.h>


/**
 * Create a new volume Processor.
 *
 * \return   The new volume Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_volume(Processor* proc);


#endif // K_PROC_VOLUME_H


