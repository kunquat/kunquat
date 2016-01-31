

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
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


#include <init/devices/Device_impl.h>

#include <stdlib.h>


#define DELAY_DEFAULT_BUF_LENGTH (2.0)
#define DELAY_MAX_BUF_LENGTH (60.0)


typedef struct Proc_delay
{
    Device_impl parent;

    double max_delay;
    double init_delay;
} Proc_delay;


/**
 * Create a new delay processor.
 *
 * \return   The new delay processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_delay(void);


#endif // K_PROC_DELAY_H


