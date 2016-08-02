

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


#ifndef KQT_PROC_ADD_H
#define KQT_PROC_ADD_H


#include <decl.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdint.h>
#include <stdlib.h>


#define ADD_TONES_MAX 32
#define ADD_BASE_FUNC_SIZE 4096


typedef struct Add_tone
{
    double pitch_factor;
    double volume_factor;
    double panning;
} Add_tone;


typedef struct Proc_add
{
    Device_impl parent;

    Sample* base;
    bool is_ramp_attack_enabled;
    bool is_rand_phase_enabled;
    Add_tone tones[ADD_TONES_MAX];
} Proc_add;


#endif // KQT_PROC_ADD_H


