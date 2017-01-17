

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


#ifndef KQT_PROC_FORCE_H
#define KQT_PROC_FORCE_H


#include <decl.h>
#include <init/devices/Device_impl.h>


typedef struct Proc_force
{
    Device_impl parent;

    double global_force;
    double force_var;

    const Envelope* force_env;
    bool is_force_env_enabled;
    bool is_force_env_loop_enabled;
    double force_env_scale_amount;
    double force_env_scale_centre;

    const Envelope* force_release_env;
    bool is_force_release_env_enabled;
    double force_release_env_scale_amount;
    double force_release_env_scale_centre;

    Envelope* def_force_release_env;

    bool is_release_ramping_enabled;
} Proc_force;


#endif // KQT_PROC_FORCE_H


