

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


#ifndef K_GENERATOR_PCM_H
#define K_GENERATOR_PCM_H


#include <stdint.h>
#include <math.h>

#include <Sample.h>
#include <Generator.h>
#include <Voice_state.h>
#include <AAtree.h>


#define PCM_SAMPLES_MAX (512)

#define PCM_SOURCES_MAX (16)
#define PCM_EXPRESSIONS_MAX (16)
#define PCM_RANDOMS_MAX (8)


typedef struct Generator_pcm
{
    Generator parent;
} Generator_pcm;


/**
 * Creates a new PCM Generator.
 *
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new PCM Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_pcm(uint32_t buffer_size,
                             uint32_t mix_rate);


/**
 * Returns PCM Generator property information.
 *
 * \param gen             The PCM Generator -- must be valid.
 * \param property_type   The property type -- must not be \c NULL.
 *
 * \return   The PCM Generator property description matching
 *           \a property_type, or \c NULL if one does not exist.
 */
char* Generator_pcm_property(Generator* gen, const char* property_type);


#endif // K_GENERATOR_PCM_H


