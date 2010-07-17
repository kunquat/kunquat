

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
#include <stdbool.h>
#include <string.h>

#include <string_common.h>
#include <xassert.h>


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


int string_extract_index(const char* path, const char* prefix, int digits)
{
    assert(path != NULL);
    assert(digits > 0);
    if (!string_has_prefix(path, prefix))
    {
        return -1;
    }
    int prefix_len = 0;
    if (prefix != NULL)
    {
        prefix_len = strlen(prefix);
    }
    else
    {
        prefix_len = strcspn(path, "_"); // FIXME: search for a hex digit instead
        if (path[prefix_len] == '\0')
        {
            return -1;
        }
        ++prefix_len;
    }
    const char* num_s = path + prefix_len;
    static const char hex_digits[] = "0123456789abcdef";
    int index = 0;
    for (int i = 0; i < digits; ++i, ++num_s)
    {
        index *= 0x10;
        if (*num_s == '\0')
        {
            return -1;
        }
        char* pos = strchr(hex_digits, *num_s);
        if (pos == NULL)
        {
            return -1;
        }
        index += pos - hex_digits;
    }
    if (*num_s != '/')
    {
        return -1;
    }
    return index;
}


