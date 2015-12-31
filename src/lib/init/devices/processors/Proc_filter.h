

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_FILTER_H
#define K_PROC_FILTER_H


#include <init/devices/Device_impl.h>


typedef struct Proc_filter
{
    Device_impl parent;
    double cutoff;
    double resonance;
} Proc_filter;


/**
 * Create a new filter processor.
 *
 * \return   The new filter processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_filter(void);


#endif // K_PROC_FILTER_H


