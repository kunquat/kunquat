

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


#ifndef KQT_PROC_SAMPLE_H
#define KQT_PROC_SAMPLE_H


#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


#define SAMPLES_MAX (512)

#define SAMPLE_RANDOMS_MAX (8)


typedef struct Proc_sample
{
    Device_impl parent;
} Proc_sample;


#endif // KQT_PROC_SAMPLE_H


