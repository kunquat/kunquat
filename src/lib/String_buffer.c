

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

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


bool String_buffer_get_error(String_buffer* sb)
{
    assert(sb != NULL);
    return sb->error;
}


long String_buffer_get_length(String_buffer* sb)
{
    assert(sb != NULL);
    return sb->length;
}


bool String_buffer_append(String_buffer* sb, char ch)
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
    return true;
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


