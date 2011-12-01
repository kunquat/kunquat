

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <Event_buffer.h>
#include <math_common.h>
#include <xassert.h>
#include <xmemory.h>


struct Event_buffer
{
    char* buf;
    int size;
    int read_pos;
    int write_pos;
};


// suggest size = 16384
Event_buffer* new_Event_buffer(int size)
{
    assert(size > 0);
    assert(size < 32768);
    Event_buffer* buf = xalloc(Event_buffer);
    if (buf == NULL)
    {
        return NULL;
    }
    buf->buf = xcalloc(char, size);
    if (buf->buf == NULL)
    {
        del_Event_buffer(buf);
        return NULL;
    }
    buf->size = size;
    buf->read_pos = 0;
    buf->write_pos = 0;
    return buf;
}


bool Event_buffer_add(Event_buffer* buf, char* event)
{
    assert(buf != NULL);
    assert(event != NULL);
    int len = strlen(event);
    assert(len < buf->size / 2); // FIXME
    bool dropped = false;
    if (buf->write_pos < buf->read_pos &&
            buf->read_pos <= buf->write_pos + 3)
    {
        dropped = true;
        Event_buffer_get(buf, NULL, 0);
        assert(buf->read_pos < buf->write_pos ||
                buf->read_pos > buf->write_pos + 3);
    }
    // 4: comma + length + trailing null
    if (buf->write_pos + len + 4 >= buf->size)
    {
        while (buf->read_pos > buf->write_pos)
        {
            dropped = true;
            Event_buffer_get(buf, NULL, 0);
        }
        buf->write_pos = 0;
    }
    while (buf->write_pos < buf->read_pos &&
            buf->read_pos < buf->write_pos + len + 4)
    {
        dropped = true;
        Event_buffer_get(buf, NULL, 0);
    }
    buf->buf[buf->write_pos] = ',';
    ++buf->write_pos;
    *(int16_t*)&buf->buf[buf->write_pos] = len;
    buf->write_pos += 2;
    assert(buf->write_pos + len < buf->size);
    strcpy(&buf->buf[buf->write_pos], event);
    buf->write_pos += len;
    buf->buf[buf->write_pos] = '\0';
    return dropped;
}


bool Event_buffer_get(Event_buffer* buf, char* dest, int size)
{
    assert(buf != NULL);
    if (buf->read_pos == buf->write_pos)
    {
        assert(buf->buf[buf->read_pos] == '\0');
        return false;
    }
    char cont = buf->buf[buf->read_pos];
    ++buf->read_pos;
    if (!cont)
    {
        assert(buf->read_pos > 1);
        buf->read_pos = 0;
        return Event_buffer_get(buf, dest, size);
    }
    int16_t len = *(int16_t*)&buf->buf[buf->read_pos];
    assert(len > 0);
    assert(len < buf->size);
    buf->read_pos += 2;
    if (dest != NULL)
    {
        assert(size > 0);
        int copy_len = MIN(len, size - 1);
        strncpy(dest, &buf->buf[buf->read_pos], copy_len);
        dest[copy_len] = '\0';
    }
    buf->read_pos += len;
    return true;
}


void Event_buffer_clear(Event_buffer* buf)
{
    assert(buf != NULL);
    buf->read_pos = buf->write_pos = 0;
    buf->buf[buf->read_pos] = '\0';
    return;
}


void del_Event_buffer(Event_buffer* buf)
{
    if (buf == NULL)
    {
        return;
    }
    xfree(buf->buf);
    xfree(buf);
    return;
}


