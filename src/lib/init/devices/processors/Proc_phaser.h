

/*
 * Author: Tomi Jylhä-Ollila, Finland 2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_PHASER_H
#define KQT_PROC_PHASER_H


#include <init/devices/Device_impl.h>


#define PHASER_STAGES_MIN 2
#define PHASER_STAGES_MAX 32

#define PHASER_NOTCH_SEP_MIN 0.01
#define PHASER_NOTCH_SEP_MAX 16


typedef struct Proc_phaser
{
    Device_impl parent;

    int stage_count;
    double cutoff;
    double notch_separation;
    double dry_wet_ratio;
} Proc_phaser;


#endif // KQT_PROC_PHASER_H


