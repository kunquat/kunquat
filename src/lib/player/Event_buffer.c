

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdio.h>
#include <string.h>

#include <memory.h>
#include <player/Event_buffer.h>
#include <xassert.h>


struct Event_buffer_2
{
    size_t size;
    size_t write_pos;
    char* buf;
};


static const char EMPTY_BUFFER[] = "[]";


Event_buffer_2* new_Event_buffer_2(size_t size)
{
    assert(size > EVENT_LEN_MAX);

    Event_buffer_2* ebuf = memory_alloc_item(Event_buffer_2);
    if (ebuf == NULL)
        return NULL;

    // Sanitise fields
    ebuf->size = size;
    ebuf->write_pos = 0;
    ebuf->buf = NULL;

    // Init fields
    ebuf->buf = memory_calloc_items(char, size + 1);
    if (ebuf->buf == NULL)
    {
        del_Event_buffer_2(ebuf);
        return NULL;
    }
    Event_buffer_2_clear(ebuf);

    return ebuf;
}


bool Event_buffer_2_is_full(const Event_buffer_2* ebuf)
{
    assert(ebuf != NULL);
    return (ebuf->write_pos >= ebuf->size - EVENT_LEN_MAX);
}


const char* Event_buffer_2_get_events(const Event_buffer_2* ebuf)
{
    assert(ebuf != NULL);
    return ebuf->buf;
}


void Event_buffer_2_add(
        Event_buffer_2* ebuf,
        int ch,
        const char* name,
        Value* arg)
{
    assert(ebuf != NULL);
    assert(!Event_buffer_2_is_full(ebuf));
    assert(ch >= 0);
    assert(ch < KQT_CHANNELS_MAX);
    assert(name != NULL);
    assert(arg != NULL);

    int advance = 0;

    // Everything before the value
    advance += sprintf(
            ebuf->buf + ebuf->write_pos + advance,
            "%s[%d, [\"%s\", ",
            ebuf->write_pos == 1 ? "" : ", ",
            ch,
            name);

    static const char closing_str[] = "]]";

    // Value
    const int max_value_bytes = EVENT_LEN_MAX - advance - strlen(closing_str);
    advance += Value_serialise(
            arg,
            max_value_bytes,
            ebuf->buf + ebuf->write_pos + advance);

    // Close the event
    advance += sprintf(
            ebuf->buf + ebuf->write_pos + advance,
            "%s",
            closing_str);

    // Update the write position
    ebuf->write_pos += advance;
    assert(ebuf->write_pos < ebuf->size);

    // Close the list
    strcpy(ebuf->buf + ebuf->write_pos, "]");

    return;
}


void Event_buffer_2_clear(Event_buffer_2* ebuf)
{
    assert(ebuf != NULL);

    strcpy(ebuf->buf, EMPTY_BUFFER);
    ebuf->write_pos = 1;

    return;
}


void del_Event_buffer_2(Event_buffer_2* ebuf)
{
    if (ebuf == NULL)
        return;

    memory_free(ebuf->buf);
    memory_free(ebuf);

    return;
}


