

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <debug/assert.h>
#include <devices/dsps/Freeverb_allpass.h>
#include <math_common.h>
#include <memory.h>


struct Freeverb_allpass
{
    kqt_frame feedback;
    kqt_frame* buffer;
    uint32_t buffer_size;
    uint32_t buffer_pos;
};


Freeverb_allpass* new_Freeverb_allpass(uint32_t buffer_size)
{
    assert(buffer_size > 0);

    Freeverb_allpass* allpass = memory_alloc_item(Freeverb_allpass);
    if (allpass == NULL)
        return NULL;

    allpass->feedback = 0;
    allpass->buffer = NULL;
    allpass->buffer_size = 0;
    allpass->buffer_pos = 0;
    allpass->buffer = memory_alloc_items(kqt_frame, buffer_size);
    if (allpass->buffer == NULL)
    {
        del_Freeverb_allpass(allpass);
        return NULL;
    }
    allpass->buffer_size = buffer_size;
    Freeverb_allpass_clear(allpass);

    return allpass;
}


void Freeverb_allpass_set_feedback(Freeverb_allpass* allpass, kqt_frame feedback)
{
    assert(allpass != NULL);
    assert(feedback > -1);
    assert(feedback < 1);
    allpass->feedback = feedback;
    return;
}


kqt_frame Freeverb_allpass_process(Freeverb_allpass* allpass, kqt_frame input)
{
    assert(allpass != NULL);

    kqt_frame bufout = allpass->buffer[allpass->buffer_pos];
    bufout = undenormalise(bufout);
    allpass->buffer[allpass->buffer_pos] = input + (bufout * allpass->feedback);

    ++allpass->buffer_pos;
    if (allpass->buffer_pos >= allpass->buffer_size)
        allpass->buffer_pos = 0;

    return -input + bufout;
}


bool Freeverb_allpass_resize_buffer(Freeverb_allpass* allpass, uint32_t new_size)
{
    assert(allpass != NULL);
    assert(new_size > 0);

    if (new_size == allpass->buffer_size)
        return true;

    kqt_frame* buffer = memory_realloc_items(kqt_frame, new_size, allpass->buffer);
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

    for (uint32_t i = 0; i < allpass->buffer_size; ++i)
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


