

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


#ifndef KQT_PROC_ENVGEN_H
#define KQT_PROC_ENVGEN_H


#include <decl.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_envgen
{
    Device_impl parent;

    bool is_time_env_enabled;
    const Envelope* time_env;
    bool is_loop_enabled;

    bool trig_immediate;
    bool trig_release;

    bool trig_impulse_floor;
    float trig_impulse_floor_on;
    float trig_impulse_floor_off;

    bool trig_impulse_ceil;
    float trig_impulse_ceil_on;
    float trig_impulse_ceil_off;

    bool is_linear_force;

    double global_adjust;

    double y_min;
    double y_max;
} Proc_envgen;


#endif // KQT_PROC_ENVGEN_H


