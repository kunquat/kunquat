

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


#ifndef KQT_PROC_FILTER_H
#define KQT_PROC_FILTER_H


#include <init/devices/Device_impl.h>


#define FILTER_DEFAULT_CUTOFF 100.0
#define FILTER_DEFAULT_RESONANCE 0.0


typedef struct Proc_filter
{
    Device_impl parent;
    double cutoff;
    double resonance;
} Proc_filter;


#endif // KQT_PROC_FILTER_H


