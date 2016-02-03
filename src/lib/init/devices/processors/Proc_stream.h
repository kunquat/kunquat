

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


#ifndef K_PROC_STREAM_H
#define K_PROC_STREAM_H


#include <decl.h>
#include <init/devices/Device_impl.h>


typedef struct Proc_stream
{
    Device_impl parent;

    double init_value;
    double init_osc_speed;
    double init_osc_depth;
} Proc_stream;


#endif // K_PROC_STREAM_H


