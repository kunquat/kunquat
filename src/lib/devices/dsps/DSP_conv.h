

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


#ifndef K_DSP_CONV_H
#define K_DSP_CONV_H


#include <stdint.h>

#include <devices/DSP.h>
#include <kunquat/limits.h>


/**
 * Creates a new convolution DSP.
 *
 * The convolution algorithm used in this DSP is only suitable for very short
 * impulse responses.
 *
 * \param buffer_size   The size of the buffers -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   The new convolution DSP if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_DSP_conv(DSP* dsp);


#endif // K_DSP_CONV_H


