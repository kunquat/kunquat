

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


#ifndef KQT_PROC_KS_H
#define KQT_PROC_KS_H


#include <init/devices/Device_impl.h>
#include <init/devices/param_types/Envelope.h>


typedef struct Proc_ks
{
    Device_impl parent;

    double damp;

    const Envelope* init_env;
    bool is_init_env_loop_enabled;
    double init_env_scale_amount;
    double init_env_scale_centre;
    Envelope* def_init_env;

    const Envelope* shift_env;
    bool is_shift_env_enabled;
    double shift_env_scale_amount;
    double shift_env_scale_centre;
    Envelope* def_shift_env;
    double shift_env_trig_threshold;
    double shift_env_strength_var;

    const Envelope* rel_env;
    bool is_rel_env_enabled;
    double rel_env_scale_amount;
    double rel_env_scale_centre;
    Envelope* def_rel_env;
    double rel_env_strength_var;
} Proc_ks;


#endif // KQT_PROC_KS_H


