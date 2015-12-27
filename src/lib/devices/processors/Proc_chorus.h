

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PROC_CHORUS_H
#define K_PROC_CHORUS_H


#include <devices/Device_impl.h>
#include <devices/Processor.h>

#include <stdlib.h>


#define CHORUS_BUF_TIME 0.25
#define CHORUS_VOICES_MAX 32

#define CHORUS_DB_MAX 18
#define CHORUS_DELAY_SCALE (1.0 / 1000.0) // delay parameter is in milliseconds
#define CHORUS_DELAY_MAX (CHORUS_BUF_TIME * 1000.0 / 2.0)


typedef struct Chorus_voice_params
{
    double delay;
    double range;
    double speed;
    double volume;
} Chorus_voice_params;


typedef struct Proc_chorus
{
    Device_impl parent;

    Chorus_voice_params voice_params[CHORUS_VOICES_MAX];
} Proc_chorus;


/**
 * Create a new chorus Processor.
 *
 * \return   The new chorus Processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_chorus(Processor* proc);


#endif // K_PROC_CHORUS_H


