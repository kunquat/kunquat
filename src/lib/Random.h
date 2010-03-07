

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


#ifndef K_RANDOM_H
#define K_RANDOM_H


#include <stdlib.h>
#include <stdint.h>


/**
 * This is a portable pseudo-random generator.
 */
typedef struct Random Random;


/**
 * Creates a new Random generator.
 *
 * \return   The new Random if successful, or \c NULL if memory allocation
 *           failed.
 */
Random* new_Random(void);


/**
 * Sets the random seed in the Random.
 *
 * \param random   The Random generator -- must not be \c NULL.
 * \param seed     The random seed.
 */
void Random_set_seed(Random* random, uint32_t seed);


/**
 * Gets a pseudo-random number from the Random generator
 *
 * \param random   The Random generator -- must not be \c NULL.
 *
 * \return   The pseudo-random number.
 */
uint32_t Random_get(Random* random);


/**
 * Destroys an existing Random generator.
 *
 * \param random   The Random generator -- must not be \c NULL.
 */
void del_Random(Random* random);


#endif // K_RANDOM_H


