

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_LOOPER_H
#define KQT_PROC_LOOPER_H


#include <init/devices/Device_impl.h>


#define LOOPER_MIN_MAX_REC_TIME 0.1
#define LOOPER_MAX_MAX_REC_TIME 60.0
#define LOOPER_DEFAULT_MAX_REC_TIME 16.0
#define LOOPER_MAX_BUF_LENGTH 60.0

#define LOOPER_DEFAULT_STATE_XFADE_TIME 0.005
#define LOOPER_DEFAULT_PLAY_XFADE_TIME 0.005
#define LOOPER_DEFAULT_MIX_XFADE_TIME 0


typedef struct Proc_looper
{
    Device_impl parent;

    double max_rec_time;

    double state_xfade_time;
    double play_xfade_time;
    double mix_xfade_time;
} Proc_looper;


#endif // KQT_PROC_LOOPER_H


