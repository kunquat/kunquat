

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Freeverb_comb.h>

#include <debug/assert.h>
#include <intrinsics.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


struct Freeverb_comb
{
    float filter_store;
    float* buffer;
    int32_t buffer_size;
    int32_t buffer_pos;
};


Freeverb_comb* new_Freeverb_comb(int32_t buffer_size)
{
    rassert(buffer_size > 0);

    Freeverb_comb* comb = memory_alloc_item(Freeverb_comb);
    if (comb == NULL)
        return NULL;

    comb->filter_store = 0;
    comb->buffer = NULL;
    comb->buffer_size = 0;
    comb->buffer_pos = 0;

    comb->buffer = memory_alloc_items(float, buffer_size);
    if (comb->buffer == NULL)
    {
        del_Freeverb_comb(comb);
        return NULL;
    }
    comb->buffer_size = buffer_size;
    Freeverb_comb_clear(comb);

    return comb;
}


void Freeverb_comb_process(
        Freeverb_comb* comb,
        float* out_buf,
        const float* in_buf,
        const float* refls,
        const float* damps,
        int32_t frame_count)
{
    rassert(comb != NULL);
    rassert(out_buf != NULL);
    rassert(in_buf != NULL);
    rassert(refls != NULL);
    rassert(damps != NULL);
    rassert(frame_count > 0);

#if KQT_SSE
    dassert(_MM_GET_FLUSH_ZERO_MODE() == _MM_FLUSH_ZERO_ON);
#endif

    const float* in = in_buf;
    float* out = out_buf;

    float* c_buffer = comb->buffer;
    int32_t c_buffer_pos = comb->buffer_pos;
    const int32_t c_buffer_size = comb->buffer_size;
    float c_filter_store = comb->filter_store;

    for (int32_t i = 0; i < frame_count; ++i)
    {
        float output = c_buffer[c_buffer_pos];
#if !KQT_SSE
        output = undenormalise(output);
#endif
        const float damp1 = damps[i];
        const float damp2 = 1 - damp1;

        const float refl = refls[i];

        c_filter_store = (output * damp2) + (c_filter_store * damp1);
#if !KQT_SSE
        c_filter_store = undenormalise(c_filter_store);
#endif
        c_buffer[c_buffer_pos] = *in + (c_filter_store * refl);

        ++in;

        *out++ += output;

        ++c_buffer_pos;
        if (c_buffer_pos >= c_buffer_size)
            c_buffer_pos = 0;
    }

    comb->buffer_pos = c_buffer_pos;
    comb->filter_store = c_filter_store;

    return;
}


bool Freeverb_comb_resize_buffer(Freeverb_comb* comb, int32_t new_size)
{
    rassert(comb != NULL);
    rassert(new_size > 0);

    if (new_size == comb->buffer_size)
        return true;

    float* buffer = memory_realloc_items(float, new_size, comb->buffer);
    if (buffer == NULL)
        return false;

    comb->buffer = buffer;
    comb->buffer_size = new_size;
    Freeverb_comb_clear(comb);
    comb->buffer_pos = 0;

    return true;
}


void Freeverb_comb_clear(Freeverb_comb* comb)
{
    rassert(comb != NULL);
    rassert(comb->buffer != NULL);

    comb->filter_store = 0;

    for (int32_t i = 0; i < comb->buffer_size; ++i)
        comb->buffer[i] = 0;

    return;
}


void del_Freeverb_comb(Freeverb_comb* comb)
{
    if (comb == NULL)
        return;

    memory_free(comb->buffer);
    memory_free(comb);

    return;
}


