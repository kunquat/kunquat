

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_PADSYNTH_H
#define K_PROC_PADSYNTH_H


#include <decl.h>
#include <init/devices/Device_impl.h>
#include <mathnum/Random.h>


typedef struct Proc_padsynth
{
    Device_impl parent;

    Random* random;
    Sample* sample;
    bool is_ramp_attack_enabled;
    bool is_stereo_enabled;
} Proc_padsynth;


#endif // K_PROC_PADSYNTH_H


