

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_FREEVERB_H
#define K_GENERATOR_FREEVERB_H


#include <stdint.h>

#include <devices/Device_impl.h>
#include <devices/Generator.h>


/**
 * Create a new Freeverb Generator.
 *
 * This is a rewrite of the Freeverb public domain reverb by Jezar at
 * Dreampoint in 2000. Unlike the original, this implementation supports
 * arbitrary audio rates.
 *
 * \return   The new Freeverb Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_freeverb(Generator* gen);


#endif // K_GENERATOR_FREEVERB_H


