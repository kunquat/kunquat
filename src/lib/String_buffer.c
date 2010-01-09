

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include <String_buffer.h>

#include <xmemory.h>


struct String_buffer
{
    char* buffer;
    long length;
    long reserved;
    bool error;
};


String_buffer* new_String_buffer(void)
{
    String_buffer* sb = xalloc(String_buffer);
    if (sb == NULL)
    {
        return NULL;
    }
    sb->buffer = xcalloc(char, 8);
    if (sb->buffer == NULL)
    {
        xfree(sb);
        return NULL;
    }
    sb->length = 0;
    sb->reserved = 8;
    sb->error = false;
    return sb;
}


bool String_buffer_error(String_buffer* sb)
{
    assert(sb != NULL);
    return sb->error;
}


long String_buffer_get_length(String_buffer* sb)
{
    assert(sb != NULL);
    return sb->length;
}


bool String_buffer_append_ch(String_buffer* sb, char ch)
{
    assert(sb != NULL);
    if (sb->error)
    {
        return false;
    }
    if (sb->length >= sb->reserved - 1)
    {
        assert(sb->length == sb->reserved - 1);
        char* new_buffer = xrealloc(char, sb->reserved * 2, sb->buffer);
        if (new_buffer == NULL)
        {
            sb->error = true;
            return false;
        }
        sb->buffer = new_buffer;
        sb->reserved *= 2;
    }
    sb->buffer[sb->length] = ch;
    sb->buffer[sb->length + 1] = '\0';
    ++sb->length;
    return true;
}


bool String_buffer_append_string(String_buffer* sb, const char* str)
{
    assert(sb != NULL);
    assert(str != NULL);
    if (sb->error)
    {
        return false;
    }
    while (*str != '\0')
    {
        String_buffer_append_ch(sb, *str);
        ++str;
    }
    return !sb->error;
}


bool String_buffer_append_int(String_buffer* sb, int32_t num)
{
    assert(sb != NULL);
    if (sb->error)
    {
        return false;
    }
    char num_buf[12] = { '\0' };
    snprintf(num_buf, 12, "%" PRId32, num);
    return String_buffer_append_string(sb, num_buf);
}


bool String_buffer_append_float(String_buffer* sb, double num)
{
    assert(sb != NULL);
    if (sb->error)
    {
        return false;
    }
    char num_buf[256] = { '\0' }; // FIXME: buffer size?
    snprintf(num_buf, 256, "%.17g", num);
    return String_buffer_append_string(sb, num_buf);
}


const char* String_buffer_get_string(String_buffer* sb)
{
    assert(sb != NULL);
    return sb->buffer;
}


char* del_String_buffer(String_buffer* sb)
{
    assert(sb != NULL);
    char* buffer = sb->buffer;
    xfree(sb);
    return buffer;
}


