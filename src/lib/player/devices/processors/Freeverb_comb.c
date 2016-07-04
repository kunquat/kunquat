

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


#include <player/devices/processors/Freeverb_comb.h>

#include <debug/assert.h>
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
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(comb != NULL);
    rassert(out_buf != NULL);
    rassert(in_buf != NULL);
    rassert(refls != NULL);
    rassert(damps != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float output = comb->buffer[comb->buffer_pos];
        output = undenormalise(output);
        const float damp1 = damps[i];
        const float damp2 = 1 - damp1;
        comb->filter_store = (output * damp2) + (comb->filter_store * damp1);
        comb->filter_store = undenormalise(comb->filter_store);
        comb->buffer[comb->buffer_pos] = in_buf[i] + (comb->filter_store * refls[i]);

        out_buf[i] += output;

        ++comb->buffer_pos;
        if (comb->buffer_pos >= comb->buffer_size)
            comb->buffer_pos = 0;
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


