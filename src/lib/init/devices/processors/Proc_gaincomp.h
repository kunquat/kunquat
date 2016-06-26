

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


#ifndef KQT_PROC_GAINCOMP_H
#define KQT_PROC_GAINCOMP_H


#include <decl.h>
#include <init/devices/Device_impl.h>

#include <stdlib.h>


typedef struct Proc_gaincomp
{
    Device_impl parent;

    bool is_map_enabled;
    const Envelope* map;
} Proc_gaincomp;


#endif // KQT_PROC_GAINCOMP_H


