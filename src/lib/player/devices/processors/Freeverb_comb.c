

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
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
        Freeverb_comb* comb_l,
        Freeverb_comb* comb_r,
        float* out_buf,
        const float* in_buf,
        const float* refls,
        const float* damps,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(comb_l != NULL);
    rassert(comb_r != NULL);
    rassert(out_buf != NULL);
    rassert(in_buf != NULL);
    rassert(refls != NULL);
    rassert(damps != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

#ifdef KQT_SSE
    dassert(_MM_GET_FLUSH_ZERO_MODE() == _MM_FLUSH_ZERO_ON);
#endif

    const float* in = in_buf + buf_start;
    float* out = out_buf + (buf_start * 2);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float output_l = comb_l->buffer[comb_l->buffer_pos];
        float output_r = comb_r->buffer[comb_r->buffer_pos];
#ifndef KQT_SSE
        output_l = undenormalise(output_l);
        output_r = undenormalise(output_r);
#endif
        const float damp1 = damps[i];
        const float damp2 = 1 - damp1;

        const float refl = refls[i];

        comb_l->filter_store = (output_l * damp2) + (comb_l->filter_store * damp1);
        comb_r->filter_store = (output_r * damp2) + (comb_r->filter_store * damp1);
#ifndef KQT_SSE
        comb_l->filter_store = undenormalise(comb_l->filter_store);
        comb_r->filter_store = undenormalise(comb_r->filter_store);
#endif
        comb_l->buffer[comb_l->buffer_pos] = *in + (comb_l->filter_store * refl);
        comb_r->buffer[comb_r->buffer_pos] = *in + (comb_r->filter_store * refl);

        ++in;

        *out++ += output_l;
        *out++ += output_r;

        ++comb_l->buffer_pos;
        if (comb_l->buffer_pos >= comb_l->buffer_size)
            comb_l->buffer_pos = 0;

        ++comb_r->buffer_pos;
        if (comb_r->buffer_pos >= comb_r->buffer_size)
            comb_r->buffer_pos = 0;
    }

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


