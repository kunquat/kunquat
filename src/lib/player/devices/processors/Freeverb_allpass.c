

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


#include <player/devices/processors/Freeverb_allpass.h>

#include <debug/assert.h>
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
    assert(buffer_size > 0);

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
    assert(allpass != NULL);
    assert(feedback > -1);
    assert(feedback < 1);
    allpass->feedback = feedback;
    return;
}


void Freeverb_allpass_process(
        Freeverb_allpass* allpass, float* buffer, int32_t buf_start, int32_t buf_stop)
{
    assert(allpass != NULL);
    assert(buffer != NULL);
    assert(buf_start >= 0);
    assert(buf_stop > buf_start);

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        float bufout = allpass->buffer[allpass->buffer_pos];
        bufout = undenormalise(bufout);
        allpass->buffer[allpass->buffer_pos] = buffer[i] + (bufout * allpass->feedback);

        buffer[i] = -buffer[i] + bufout;

        ++allpass->buffer_pos;
        if (allpass->buffer_pos >= allpass->buffer_size)
            allpass->buffer_pos = 0;
    }

    return;
}


bool Freeverb_allpass_resize_buffer(Freeverb_allpass* allpass, int32_t new_size)
{
    assert(allpass != NULL);
    assert(new_size > 0);

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
    assert(allpass != NULL);
    assert(allpass->buffer != NULL);

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


