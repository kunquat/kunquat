

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_PCM_H
#define K_GENERATOR_PCM_H


#include <stdint.h>
#include <math.h>

#include <containers/AAtree.h>
#include <devices/Device_impl.h>
#include <devices/Generator.h>


#define PCM_SAMPLES_MAX (512)

#define PCM_SOURCES_MAX (16)
#define PCM_EXPRESSIONS_MAX (16)
#define PCM_RANDOMS_MAX (8)


/**
 * Creates a new PCM Generator.
 *
 * \return   The new PCM Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_impl* new_Generator_pcm(Generator* gen);


/**
 * Returns PCM Generator property information.
 *
 * \param gen             The PCM Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The PCM Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
const char* Generator_pcm_property(Generator* gen, const char* property_type);


#endif // K_GENERATOR_PCM_H


