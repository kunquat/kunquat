

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

#include <DSP_buffer.h>
#include <kunquat/limits.h>
#include <math_common.h>

#include <xmemory.h>


struct DSP_buffer
{
    uint32_t size;
    kqt_frame* bufs[KQT_BUFFERS_MAX];
};


DSP_buffer* new_DSP_buffer(uint32_t size)
{
    assert(size > 0);
    assert(size <= 4194304);
    DSP_buffer* buffer = xalloc(DSP_buffer);
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
            del_DSP_buffer(buffer);
            return NULL;
        }
    }
    return buffer;
}


uint32_t DSP_buffer_get_size(DSP_buffer* buffer)
{
    assert(buffer != NULL);
    return buffer->size;
}


bool DSP_buffer_resize(DSP_buffer* buffer, uint32_t size)
{
    assert(buffer != NULL);
    assert(size > 0);
    assert(size <= 4194304);
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


void DSP_buffer_clear(DSP_buffer* buffer)
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


void DSP_buffer_mix(DSP_buffer* buffer, DSP_buffer* in)
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


kqt_frame* DSP_buffer_get_buffer(DSP_buffer* buffer, int index)
{
    assert(buffer != NULL);
    assert(index >= 0);
    assert(index < KQT_BUFFERS_MAX);
    return buffer->bufs[index];
}


void del_DSP_buffer(DSP_buffer* buffer)
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


