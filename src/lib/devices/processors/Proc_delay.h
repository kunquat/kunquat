

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


#ifndef K_PROC_DELAY_H
#define K_PROC_DELAY_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


/**
 * Create a new delay Processor.
 *
 * This Processor implements a tapped delay line (TDL).
 *
 * \return   The new delay Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_delay(Processor* proc);


#endif // K_PROC_DELAY_H


