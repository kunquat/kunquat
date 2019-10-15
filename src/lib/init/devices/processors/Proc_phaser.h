

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


#define PHASER_STAGES_MIN 1
#define PHASER_STAGES_MAX 32
#define PHASER_STAGES_DEFAULT 2

#define PHASER_BANDWIDTH_MIN 0.125
#define PHASER_BANDWIDTH_MAX 12.0
#define PHASER_BANDWIDTH_DEFAULT 2


typedef struct Proc_phaser
{
    Device_impl parent;

    int stage_count;
    double cutoff;
    double bandwidth;
    double dry_wet_ratio;
} Proc_phaser;


#endif // KQT_PROC_PHASER_H


