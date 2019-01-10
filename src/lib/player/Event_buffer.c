

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Event_buffer.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Event_buffer
{
    int32_t size;
    int32_t write_pos;
    char* buf;

    int32_t events_added;
    bool is_skipping;
    int32_t events_skipped;
};


static const char EMPTY_BUFFER[] = "[]";


Event_buffer* new_Event_buffer(int32_t size)
{
    rassert(size >= 0);
    rassert(size <= EVENT_BUF_SIZE_MAX);

    Event_buffer* ebuf = memory_alloc_item(Event_buffer);
    if (ebuf == NULL)
        return NULL;

    // Sanitise fields
    ebuf->size = max((int32_t)strlen(EMPTY_BUFFER) + 1, size);
    ebuf->write_pos = 0;
    ebuf->buf = NULL;

    ebuf->events_added = 0;
    ebuf->is_skipping = false;
    ebuf->events_skipped = 0;

    // Init fields
    ebuf->buf = memory_calloc_items(char, ebuf->size + 1);
    if (ebuf->buf == NULL)
    {
        del_Event_buffer(ebuf);
        return NULL;
    }
    Event_buffer_clear(ebuf);

    return ebuf;
}


bool Event_buffer_is_empty(const Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    return string_eq(ebuf->buf, EMPTY_BUFFER);
}


bool Event_buffer_is_full(const Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    return (ebuf->size < EVENT_LEN_MAX) ||
        (ebuf->write_pos >= ebuf->size - EVENT_LEN_MAX);
}


void Event_buffer_reset_add_counter(Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    rassert(!ebuf->is_skipping);

    ebuf->events_added = 0;

    return;
}


void Event_buffer_start_skipping(Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    rassert(!ebuf->is_skipping);

    ebuf->is_skipping = true;
    ebuf->events_skipped = 0;

    return;
}


bool Event_buffer_is_skipping(const Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    return ebuf->is_skipping;
}


bool Event_buffer_is_zero_skipping(const Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    return ebuf->is_skipping && (ebuf->events_added == 0);
}


const char* Event_buffer_get_events(const Event_buffer* ebuf)
{
    rassert(ebuf != NULL);
    return ebuf->buf;
}


void Event_buffer_add(Event_buffer* ebuf, int ch, const char* name, const Value* arg)
{
    rassert(ebuf != NULL);
    rassert(!Event_buffer_is_full(ebuf));
    rassert(ch >= 0);
    rassert(ch < KQT_CHANNELS_MAX);
    rassert(name != NULL);
    rassert(arg != NULL);

    if (ebuf->is_skipping && (ebuf->events_added == 0))
    {
        rassert(ebuf->events_skipped == 0);
        ebuf->is_skipping = false;
    }

    // Skipping mode
    if (ebuf->is_skipping)
    {
        // This event has already been processed
        rassert(ebuf->events_skipped < ebuf->events_added);

        ++ebuf->events_skipped;
        if (ebuf->events_skipped >= ebuf->events_added)
        {
            rassert(ebuf->events_skipped == ebuf->events_added);
            ebuf->is_skipping = false;
        }

        return;
    }

    int advance = 0;

    // Everything before the name
    advance += sprintf(
            ebuf->buf + ebuf->write_pos + advance,
            "%s[%d, [",
            ebuf->write_pos == 1 ? "" : ", ",
            ch);

    // Name
    const size_t len = strlen(name);
    rassert(len > 0);
    if (name[len - 1] == '"')
    {
        // Print with properly escaped trailing double quote
        advance += sprintf(
                ebuf->buf + ebuf->write_pos + advance,
                "\"%.*s\\\"\", ",
                (int)(len - 1),
                name);
    }
    else
    {
        // Print name as-is
        advance += sprintf(
                ebuf->buf + ebuf->write_pos + advance,
                "\"%s\", ",
                name);
    }

    static const char closing_str[] = "]]";

    // Value
    const int max_value_bytes = EVENT_LEN_MAX - advance - (int)strlen(closing_str);
    advance += Value_serialise(
            arg, max_value_bytes, ebuf->buf + ebuf->write_pos + advance);

    // Close the event
    advance += sprintf(ebuf->buf + ebuf->write_pos + advance, "%s", closing_str);

    // Update the write position
    ebuf->write_pos += advance;
    rassert(ebuf->write_pos < ebuf->size);

    // Close the list
    strcpy(ebuf->buf + ebuf->write_pos, "]");

    ++ebuf->events_added;

    return;
}


void Event_buffer_clear(Event_buffer* ebuf)
{
    rassert(ebuf != NULL);

    strcpy(ebuf->buf, EMPTY_BUFFER);
    ebuf->write_pos = 1;

    return;
}


void del_Event_buffer(Event_buffer* ebuf)
{
    if (ebuf == NULL)
        return;

    memory_free(ebuf->buf);
    memory_free(ebuf);

    return;
}


