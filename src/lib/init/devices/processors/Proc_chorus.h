

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


#ifndef K_PROC_CHORUS_H
#define K_PROC_CHORUS_H


#include <init/devices/Device_impl.h>

#include <stdlib.h>


/*
#define CHORUS_BUF_TIME 0.25
#define CHORUS_VOICES_MAX 32

#define CHORUS_DB_MAX 18
#define CHORUS_DELAY_SCALE (1.0 / 1000.0) // delay parameter is in milliseconds
#define CHORUS_DELAY_MAX (CHORUS_BUF_TIME * 1000.0 / 2.0)
// */

#define DELAY_DEFAULT_BUF_LENGTH (2.0)
#define DELAY_MAX_BUF_LENGTH (60.0)


/*
typedef struct Chorus_voice_params
{
    double delay;
    double range;
    double speed;
    double volume;
} Chorus_voice_params;
// */


typedef struct Proc_chorus
{
    Device_impl parent;

    double max_delay;
    double init_delay;
} Proc_chorus;


/**
 * Create a new chorus processor.
 *
 * \return   The new chorus processor if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Proc_chorus(void);


#endif // K_PROC_CHORUS_H


