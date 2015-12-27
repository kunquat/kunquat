

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


#ifndef K_PROC_DEBUG_H
#define K_PROC_DEBUG_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_debug
{
    Device_impl parent;
    bool single_pulse;
} Proc_debug;


/**
 * Create a new Debug Processor.
 *
 * The Debug Processor generates a narrow pulse wave (with one sample value 1,
 * the rest are 0.5) that lasts no more than 10 phase cycles. Note off lasts
 * no more than two phase cycles with all sample values negated.
 *
 * \return   The new Debug Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_debug(Processor* proc);


#endif // K_PROC_DEBUG_H


