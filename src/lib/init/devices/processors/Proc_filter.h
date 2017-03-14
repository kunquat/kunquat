

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2017
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


typedef enum
{
    FILTER_TYPE_LOWPASS = 0,
    FILTER_TYPE_HIGHPASS,
    FILTER_TYPE_COUNT,
} Filter_type;


typedef struct Proc_filter
{
    Device_impl parent;
    Filter_type type;
    double cutoff;
    double resonance;
} Proc_filter;


#endif // KQT_PROC_FILTER_H


