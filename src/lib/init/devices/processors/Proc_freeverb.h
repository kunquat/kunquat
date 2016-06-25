

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_FREEVERB_H
#define KQT_PROC_FREEVERB_H


#include <init/devices/Device_impl.h>

#include <stdlib.h>


typedef struct Proc_freeverb
{
    Device_impl parent;

    double gain;
    double wet;
    double wet1;
    double wet2;
    double width;
    double reflect_setting;
    double damp_setting;
} Proc_freeverb;


#endif // KQT_PROC_FREEVERB_H


