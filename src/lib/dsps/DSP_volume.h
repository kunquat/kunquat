

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


#ifndef K_DSP_VOLUME_H
#define K_DSP_VOLUME_H


#include <stdint.h>


/**
 * Creates a new volume DSP.
 *
 * \param buffer_size   The size of the buffers -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   The new volume DSP if successful, or \c NULL if memory allocation
 *           failed.
 */
DSP* new_DSP_volume(uint32_t buffer_size);


#endif // K_DSP_VOLUME_H


