

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <kunquat/defaults.h>

#include <Handle_private.h>
#include <string/common.h>
#include <string/key_pattern.h>


static const struct
{
    const char* keyp;
    const char* def_val;
} keys_to_defaults[] =
{
#define MODULE_KEYP(name, keyp, version, def_val) { keyp, def_val },
#include <init/module_key_patterns.h>
    { NULL, NULL }
};


static const char* empty_val = "";


const char* kqt_get_default_value(const char* key)
{
    if (key == NULL)
    {
        Handle_set_error(NULL, ERROR_ARGUMENT, "key argument must not be NULL");
        return empty_val;
    }

    // Get key pattern
    char key_pattern[KQT_KEY_LENGTH_MAX] = "";
    Key_indices key_indices = { 0 };
    for (int i = 0; i < KEY_INDICES_MAX; ++i)
        key_indices[i] = -1;

    if (!extract_key_pattern(key, key_pattern, key_indices))
    {
        Handle_set_error(NULL, ERROR_ARGUMENT, "key is not well-formed");
        return empty_val;
    }

    // Find matching default value
    for (int i = 0; keys_to_defaults[i].keyp != NULL; ++i)
    {
        if (string_eq(key_pattern, keys_to_defaults[i].keyp))
            return keys_to_defaults[i].def_val;
    }

    return empty_val;
}


