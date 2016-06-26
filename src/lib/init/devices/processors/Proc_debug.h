

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


#ifndef KQT_PROC_DEBUG_H
#define KQT_PROC_DEBUG_H


#include <init/devices/Device_impl.h>

#include <stdlib.h>


typedef struct Proc_debug
{
    Device_impl parent;
    bool single_pulse;
} Proc_debug;


#endif // KQT_PROC_DEBUG_H


