

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
    uint32_t seed;
    uint32_t state;
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


void Random_set_seed(Random* random, uint32_t seed)
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


uint32_t Random_get(Random* random)
{
    assert(random != NULL);
    random->state = 1664525UL * random->state + 1013904223UL;
    return random->state;
}


void del_Random(Random* random)
{
    assert(random != NULL);
    xfree(random);
    return;
}


