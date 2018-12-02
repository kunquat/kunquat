

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


#include <player/devices/processors/Freeverb_allpass.h>

#include <debug/assert.h>
#include <intrinsics.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Freeverb_allpass
{
    float feedback;
    float* buffer;
    int32_t buffer_size;
    int32_t buffer_pos;
};


Freeverb_allpass* new_Freeverb_allpass(int32_t buffer_size)
{
    rassert(buffer_size > 0);

    Freeverb_allpass* allpass = memory_alloc_item(Freeverb_allpass);
    if (allpass == NULL)
        return NULL;

    allpass->feedback = 0;
    allpass->buffer = NULL;
    allpass->buffer_size = 0;
    allpass->buffer_pos = 0;
    allpass->buffer = memory_alloc_items(float, buffer_size);
    if (allpass->buffer == NULL)
    {
        del_Freeverb_allpass(allpass);
        return NULL;
    }
    allpass->buffer_size = buffer_size;
    Freeverb_allpass_clear(allpass);

    return allpass;
}


void Freeverb_allpass_set_feedback(Freeverb_allpass* allpass, float feedback)
{
    rassert(allpass != NULL);
    rassert(feedback > -1);
    rassert(feedback < 1);
    allpass->feedback = feedback;
    return;
}


void Freeverb_allpass_process(
        Freeverb_allpass* allpass_l,
        Freeverb_allpass* allpass_r,
        float* buffer,
        int32_t frame_count)
{
    rassert(allpass_l != NULL);
    rassert(allpass_r != NULL);
    rassert(buffer != NULL);
    rassert(frame_count > 0);

#if KQT_SSE
    dassert(_MM_GET_FLUSH_ZERO_MODE() == _MM_FLUSH_ZERO_ON);
#endif

    float* value = buffer;

    for (int32_t i = 0; i < frame_count; ++i)
    {
        float bufout_l = allpass_l->buffer[allpass_l->buffer_pos];
        float bufout_r = allpass_r->buffer[allpass_r->buffer_pos];
#if !KQT_SSE
        bufout_l = undenormalise(bufout_l);
        bufout_r = undenormalise(bufout_r);
#endif
        float value_l = *value;
        allpass_l->buffer[allpass_l->buffer_pos] =
            value_l + (bufout_l * allpass_l->feedback);
        value_l = -value_l + bufout_l;
        *value++ = value_l;

        float value_r = *value;
        allpass_r->buffer[allpass_r->buffer_pos] =
            value_r + (bufout_r * allpass_r->feedback);
        value_r = -value_r + bufout_r;
        *value++ = value_r;

        ++allpass_l->buffer_pos;
        if (allpass_l->buffer_pos >= allpass_l->buffer_size)
            allpass_l->buffer_pos = 0;

        ++allpass_r->buffer_pos;
        if (allpass_r->buffer_pos >= allpass_r->buffer_size)
            allpass_r->buffer_pos = 0;
    }

    return;
}


bool Freeverb_allpass_resize_buffer(Freeverb_allpass* allpass, int32_t new_size)
{
    rassert(allpass != NULL);
    rassert(new_size > 0);

    if (new_size == allpass->buffer_size)
        return true;

    float* buffer = memory_realloc_items(float, new_size, allpass->buffer);
    if (buffer == NULL)
        return false;

    allpass->buffer = buffer;
    allpass->buffer_size = new_size;
    Freeverb_allpass_clear(allpass);
    allpass->buffer_pos = 0;

    return true;
}


void Freeverb_allpass_clear(Freeverb_allpass* allpass)
{
    rassert(allpass != NULL);
    rassert(allpass->buffer != NULL);

    for (int32_t i = 0; i < allpass->buffer_size; ++i)
        allpass->buffer[i] = 0;

    return;
}


void del_Freeverb_allpass(Freeverb_allpass* allpass)
{
    if (allpass == NULL)
        return;

    memory_free(allpass->buffer);
    memory_free(allpass);

    return;
}


