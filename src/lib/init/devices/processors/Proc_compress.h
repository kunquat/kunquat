

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


#ifndef KQT_PROC_COMPRESS_H
#define KQT_PROC_COMPRESS_H


#include <init/devices/Device_impl.h>


typedef struct Proc_compress
{
    Device_impl parent;

    double attack;
    double release;

    bool upward_enabled;
    double upward_threshold;
    double upward_ratio;

    bool downward_enabled;
    double downward_threshold;
    double downward_ratio;
} Proc_compress;


#endif // KQT_PROC_COMPRESS_H


