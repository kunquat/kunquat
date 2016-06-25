

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


#ifndef KQT_PROC_DELAY_H
#define KQT_PROC_DELAY_H


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


#endif // KQT_PROC_DELAY_H


