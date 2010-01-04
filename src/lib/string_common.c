

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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
#include <string.h>

#include <string_common.h>


bool string_has_prefix(const char* str, const char* prefix)
{
    if (prefix == NULL || *prefix == '\0')
    {
        return true;
    }
    if (str == NULL || *str == '\0')
    {
        return false;
    }
    return strncmp(str, prefix, strlen(prefix)) == 0;
}


bool string_has_suffix(const char* str, const char* suffix)
{
    if (suffix == NULL || *suffix == '\0')
    {
        return true;
    }
    if (str == NULL || *str == '\0')
    {
        return false;
    }
    if (strlen(suffix) > strlen(str))
    {
        return false;
    }
    const char* search = str + (strlen(str) - strlen(suffix));
    assert(strlen(search) == strlen(suffix));
    return strcmp(search, suffix) == 0;
}


