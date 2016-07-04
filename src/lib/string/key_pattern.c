

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <string/key_pattern.h>

#include <debug/assert.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static int32_t extract_num(
        const char* section, size_t section_length, size_t* digit_count)
{
    rassert(section != NULL);
    rassert(digit_count != NULL);

    int32_t num = 0;
    int mul = 1;

    *digit_count = 0;

    for (int pos = (int)section_length - 1; pos >= 0; --pos)
    {
        static const char* hexdigits = "0123456789abcdef";
        const char* digit_pos = strchr(hexdigits, section[pos]);
        if (digit_pos == NULL)
            break;

        num += (int32_t)(digit_pos - hexdigits) * mul;

        mul <<= 4;
        ++(*digit_count);
    }

    if ((*digit_count == 0) || (*digit_count == section_length))
        return -1;

    return num;
}


bool extract_key_pattern(const char* key, char* key_pattern, Key_indices indices)
{
    rassert(key != NULL);
    rassert(strlen(key) < KQT_KEY_LENGTH_MAX);
    rassert(key_pattern != NULL);
    rassert(indices != NULL);

    int next_index_pos = 0;
    char* keyp_write_pos = key_pattern;

    const char* section = key;
    size_t section_length = strcspn(section, "/");

    // FIXME: does not support a key prefix that ends with a number
    while (section[section_length] != '\0')
    {
        // Check if there's a number at the end of the section
        size_t digit_count = 0;
        const int32_t num = extract_num(section, section_length, &digit_count);

        if (digit_count > KEY_INDEX_DIGITS_MAX)
            return false;

        if (num >= 0)
        {
            // Store the number
            if (next_index_pos >= KEY_INDICES_MAX)
                return false;

            indices[next_index_pos] = num;
            ++next_index_pos;

            // Create a key pattern section of format "blabla_XXX/"
            rassert(digit_count > 0);
            rassert(digit_count <= section_length);
            const size_t prefix_length = section_length - digit_count;
            strncpy(keyp_write_pos, section, prefix_length);
            memset(keyp_write_pos + prefix_length, 'X', digit_count);
            keyp_write_pos[section_length] = '/';
        }
        else
        {
            // Copy the section as-is
            strncpy(keyp_write_pos, section, section_length);
            keyp_write_pos[section_length] = '/';
        }

        keyp_write_pos += section_length + 1;
        section = section + section_length + 1;
        section_length = strcspn(section, "/");
    }

    // Copy the last part of the key
    strncpy(keyp_write_pos, section, section_length);
    keyp_write_pos[section_length] = '\0';
    rassert((int)strlen(key) == (keyp_write_pos + strlen(keyp_write_pos) - key_pattern));

    return true;
}


