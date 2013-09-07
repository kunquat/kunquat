

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_VOLUME_H
#define K_DSP_VOLUME_H


#include <stdint.h>

#include <DSP.h>
#include <kunquat/limits.h>


/**
 * Creates a new volume DSP.
 *
 * \return   The new volume DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
Device_impl* new_DSP_volume(DSP* dsp);


#endif // K_DSP_VOLUME_H


