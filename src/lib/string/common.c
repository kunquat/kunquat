

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string/common.h>

#include <debug/assert.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool string_eq(const char* str1, const char* str2)
{
    if (str1 == NULL || *str1 == '\0')
        return (str2 == NULL) || (*str2 == '\0');

    if (str2 == NULL || *str2 == '\0')
        return false;

    return strcmp(str1, str2) == 0;
}


bool string_has_prefix(const char* str, const char* prefix)
{
    if (string_eq(prefix, ""))
        return true;

    if (string_eq(str, ""))
        return false;

    return strncmp(str, prefix, strlen(prefix)) == 0;
}


bool string_has_suffix(const char* str, const char* suffix)
{
    if (string_eq(suffix, ""))
        return true;

    if (string_eq(str, ""))
        return false;

    if (strlen(suffix) > strlen(str))
        return false;

    const char* search = str + (strlen(str) - strlen(suffix));
    assert(strlen(search) == strlen(suffix));

    return string_eq(search, suffix);
}


int string_extract_index(
        const char* path, const char* prefix, int digits, const char* after)
{
    assert(path != NULL);
    assert(digits > 0);

    if (!string_has_prefix(path, prefix))
        return -1;

    static const char hex_digits[] = "0123456789abcdef";
    int prefix_len = 0;
    if (prefix != NULL)
    {
        prefix_len = (int)strlen(prefix);
    }
    else
    {
        prefix_len = (int)strcspn(path, hex_digits);
        if (path[prefix_len] == '\0')
            return -1;
    }

    const char* num_s = path + prefix_len;
    int index = 0;
    for (int i = 0; i < digits; ++i, ++num_s)
    {
        index *= 0x10;

        if (*num_s == '\0')
            return -1;

        char* pos = strchr(hex_digits, *num_s);
        if (pos == NULL)
            return -1;

        index += (int)(pos - hex_digits);
    }

    if (string_has_prefix(after, ".") && !string_eq(num_s, after))
        return -1;
    else if ((after != NULL) && string_eq(after, "") && !string_eq(num_s, ""))
        return -1;
    else if (!string_has_prefix(num_s, after))
        return -1;

    return index;
}


