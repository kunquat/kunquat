

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
#include <assert.h>

#include <Audio_buffer.h>
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>


struct Audio_buffer
{
    uint32_t size;
    kqt_frame* bufs[KQT_BUFFERS_MAX];
};


Audio_buffer* new_Audio_buffer(uint32_t size)
{
    assert(size > 0);
    assert(size <= KQT_BUFFER_SIZE_MAX);
    Audio_buffer* buffer = xalloc(Audio_buffer);
    if (buffer == NULL)
    {
        return NULL;
    }
    buffer->size = size;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        buffer->bufs[i] = NULL;
    }
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        buffer->bufs[i] = xnalloc(kqt_frame, size);
        if (buffer->bufs[i] == NULL)
        {
            del_Audio_buffer(buffer);
            return NULL;
        }
    }
    return buffer;
}


uint32_t Audio_buffer_get_size(Audio_buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->size;
}


bool Audio_buffer_resize(Audio_buffer* buffer, uint32_t size)
{
    assert(buffer != NULL);
    assert(size > 0);
    assert(size <= KQT_BUFFER_SIZE_MAX);
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        kqt_frame* new_buf = xrealloc(kqt_frame, size, buffer->bufs[i]);
        if (new_buf == NULL)
        {
            buffer->size = MIN(buffer->size, size);
            return false;
        }
        buffer->bufs[i] = new_buf;
    }
    buffer->size = size;
    return true;
}


void Audio_buffer_clear(Audio_buffer* buffer)
{
    assert(buffer != NULL);
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        for (uint32_t k = 0; k < buffer->size; ++k)
        {
            buffer->bufs[i][k] = 0;
        }
    }
    return;
}


void Audio_buffer_mix(Audio_buffer* buffer, Audio_buffer* in)
{
    assert(buffer != NULL);
    assert(in != NULL);
    assert(buffer->size == in->size);
    if (buffer == in)
    {
        return;
    }
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        for (uint32_t k = 0; k < buffer->size; ++k)
        {
            buffer->bufs[i][k] += in->bufs[i][k];
        }
    }
    return;
}


kqt_frame* Audio_buffer_get_buffer(Audio_buffer* buffer, int index)
{
    assert(buffer != NULL);
    assert(index >= 0);
    assert(index < KQT_BUFFERS_MAX);
    return buffer->bufs[index];
}


void del_Audio_buffer(Audio_buffer* buffer)
{
    assert(buffer != NULL);
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        if (buffer->bufs[i] != NULL)
        {
            xfree(buffer->bufs[i]);
        }
    }
    xfree(buffer);
    return;
}


