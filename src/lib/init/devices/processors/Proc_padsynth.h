

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_PADSYNTH_H
#define KQT_PROC_PADSYNTH_H


#include <decl.h>
#include <init/devices/Device_impl.h>
#include <mathnum/Random.h>

#include <stdbool.h>
#include <stdint.h>


typedef struct Padsynth_sample_map Padsynth_sample_map;


typedef struct Padsynth_sample_entry
{
    double centre_pitch;
    Sample* sample;
} Padsynth_sample_entry;


const Padsynth_sample_entry* Padsynth_sample_map_get_entry(
        const Padsynth_sample_map* sm, double pitch);


int32_t Padsynth_sample_map_get_sample_length(const Padsynth_sample_map* sm);


typedef struct Proc_padsynth
{
    Device_impl parent;

    Random random;
    Padsynth_sample_map* sample_map;
    bool is_ramp_attack_enabled;
    bool is_stereo_enabled;
} Proc_padsynth;


#endif // KQT_PROC_PADSYNTH_H


