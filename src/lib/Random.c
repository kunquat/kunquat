

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


#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <Random.h>

#include <xmemory.h>


struct Random
{
    uint64_t seed;
    uint64_t state;
};


Random* new_Random(void)
{
    Random* random = xalloc(Random);
    if (random == NULL)
    {
        return NULL;
    }
    random->seed = random->state = 1;
    return random;
}


void Random_set_seed(Random* random, uint64_t seed)
{
    assert(random != NULL);
    random->seed = random->state = seed;
    return;
}


void Random_reset(Random* random)
{
    assert(random != NULL);
    random->state = random->seed;
    return;
}


uint64_t Random_get_uint64(Random* random)
{
    assert(random != NULL);
    // multiplier and increment from Knuth
    random->state = 6364136223846793005ULL * random->state + 1442695040888963407ULL;
    return random->state;
}


uint32_t Random_get_uint32(Random* random)
{
    assert(random != NULL);
    return Random_get_uint64(random) >> 32;
}


int32_t Random_get_index(Random* random, int32_t size)
{
    assert(random != NULL);
    assert(size > 0);
    return (int32_t)(Random_get_uint64(random) >> 33) % size;
}


double Random_get_float_scale(Random* random)
{
    assert(random != NULL);
    return Random_get_uint64(random) / (double)KQT_RANDOM64_MAX;
}


double Random_get_float_signal(Random* random)
{
    assert(random != NULL);
    uint64_t bits = (Random_get_uint64(random) >> 1); // max: 0x7fffffffffffffff
    bits &= ~(uint64_t)1;                             //      0x7ffffffffffffffe
    return ((int64_t)bits - 0x3fffffffffffffffLL) /
                    (double)0x3fffffffffffffffLL;
}


void del_Random(Random* random)
{
    assert(random != NULL);
    xfree(random);
    return;
}


