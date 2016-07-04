

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <mathnum/Random.h>

#include <debug/assert.h>
#include <mathnum/hmac.h>

#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define EXCESS_DOUBLE_BITS (64 - DBL_MANT_DIG)
#define DOUBLE_LIMIT ((int64_t)1 << DBL_MANT_DIG)


Random* Random_init(Random* random, const char* context)
{
    rassert(random != NULL);
    rassert(context != NULL);
    rassert(strlen(context) <= CONTEXT_LEN_MAX);

    strcpy(random->context, context);

    Random_set_seed(random, 1);

    return random;
}


void Random_set_seed(Random* random, uint64_t seed)
{
    rassert(random != NULL);

    uint64_t cseed = 0;
    uint64_t dummy = 0;
    hmac_md5(seed, random->context, &cseed, &dummy);

    random->seed = random->state = cseed;

    return;
}


void Random_reset(Random* random)
{
    rassert(random != NULL);
    random->state = random->seed;
    return;
}


uint64_t Random_get_uint64(Random* random)
{
    rassert(random != NULL);

    // multiplier and increment from Knuth
    random->state = 6364136223846793005ULL * random->state +
                    1442695040888963407ULL;

    return random->state;
}


uint32_t Random_get_uint32(Random* random)
{
    rassert(random != NULL);
    return (uint32_t)(Random_get_uint64(random) >> 32);
}


double Random_get_float_lb(Random* random)
{
    rassert(random != NULL);
    return
        (double)(Random_get_uint64(random) >> EXCESS_DOUBLE_BITS) / (double)DOUBLE_LIMIT;
}


int32_t Random_get_index(Random* random, int32_t size)
{
    rassert(random != NULL);
    rassert(size > 0);

    return (int32_t)(Random_get_uint64(random) >> 33) % size;
}


double Random_get_float_scale(Random* random)
{
    rassert(random != NULL);
    return
        (double)(Random_get_uint64(random) >> EXCESS_DOUBLE_BITS) /
        (double)(DOUBLE_LIMIT - 1);
}


double Random_get_float_signal(Random* random)
{
    rassert(random != NULL);

    static const int64_t max_val_abs = (DOUBLE_LIMIT >> 1) - 1;

    // Get random value in range [-max_val_abs, max_val_abs]
    int64_t bits = (int64_t)(Random_get_uint64(random) >> EXCESS_DOUBLE_BITS);
    bits &= ~(int64_t)1;
    bits -= max_val_abs;

    return (double)bits / (double)max_val_abs;
}


