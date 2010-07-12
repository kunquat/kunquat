

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_SCALE_H
#define K_DSP_SCALE_H


#include <stdint.h>


/**
 * Creates a new scale DSP.
 *
 * \param buffer_size   The size of the buffers -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   The new scale DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP* new_DSP_scale(uint32_t buffer_size);


#endif // K_DSP_SCALE_H


