

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_CHORUS_H
#define K_DSP_CHORUS_H


#include <stdint.h>

#include <devices/DSP.h>
#include <kunquat/limits.h>


/**
 * Creates a new chorus DSP.
 *
 * \return   The new chorus DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
Device_impl* new_DSP_chorus(DSP* dsp);


#endif // K_DSP_CHORUS_H


