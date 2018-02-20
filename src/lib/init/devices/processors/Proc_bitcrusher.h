

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_BITCRUSHER_H
#define KQT_PROC_BITCRUSHER_H


#include <init/devices/Device_impl.h>


#define BITCRUSHER_DEFAULT_CUTOFF 100.0
#define BITCRUSHER_DEFAULT_RESOLUTION 16.0
#define BITCRUSHER_DEFAULT_RES_IGNORE_MIN 16.0


typedef struct Proc_bitcrusher
{
    Device_impl parent;
    double cutoff;
    double resolution;
    double res_ignore_min;
} Proc_bitcrusher;


#endif // KQT_PROC_BITCRUSHER_H


