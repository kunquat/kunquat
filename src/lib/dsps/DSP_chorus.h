

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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

#include <DSP.h>
#include <kunquat/limits.h>


/**
 * Creates a new chorus DSP.
 *
 * \param buffer_size   The size of the buffers -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new chorus DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP* new_DSP_chorus(uint32_t buffer_size, uint32_t mix_rate);


#endif // K_DSP_CHORUS_H


