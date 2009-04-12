

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>

#include "Generator.h"

#include <xmemory.h>


#if 0
bool Generator_init(Generator* gen, Gen_type type, Instrument_params* ins_params)
{
    assert(gen != NULL);
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    assert(ins_params != NULL);
    gen->type = type;
    gen->init = NULL;
    gen->init_state = NULL;
    gen->uninit = NULL;
    gen->mix = NULL;
    switch (type)
    {
        case GEN_TYPE_DEBUG:
            gen->mix = Generator_debug_mix;
            break;
        case GEN_TYPE_SINE:
            gen->mix = Generator_sine_mix;
            gen->init_state = Voice_state_sine_init;
            break;
        case GEN_TYPE_PCM:
            gen->mix = Generator_pcm_mix;
            gen->init = Generator_pcm_init;
            gen->uninit = Generator_pcm_uninit;
            break;
        default:
            assert(false);
    }
    assert(gen->mix != NULL);
    assert((gen->init == NULL) == (gen->uninit == NULL));
    if (gen->init != NULL)
    {
        if (gen->init(gen) != 0)
        {
            xfree(gen);
            return NULL;
        }
    }
    return gen;
}
#endif


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    gen->mix(gen, state, nframes, offset, freq);
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


