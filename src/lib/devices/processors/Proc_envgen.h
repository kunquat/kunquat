

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_ENVGEN_H
#define K_PROC_ENVGEN_H


#include <decl.h>
#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


typedef struct Proc_envgen
{
    Device_impl parent;

    double scale;

    bool is_time_env_enabled;
    const Envelope* time_env;
    bool is_loop_enabled;
    double env_scale_amount;
    double env_scale_center;

    bool is_force_env_enabled;
    const Envelope* force_env;

    double y_min;
    double y_max;
} Proc_envgen;


/**
 * Create a new envelope generator Processor.
 *
 * \return   The new envelope generator Processor if successful, or \c NULL if
 *           memory allocation failed.
 */
Device_impl* new_Proc_envgen(Processor* proc);


/**
 * Return envelope generator Processor property information.
 *
 * \param proc            The envelope generator Processor -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 */
const char* Proc_envgen_property(const Processor* proc, const char* property_type);


#endif // K_PROC_ENVGEN_H


