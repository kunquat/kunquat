

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
#include <string.h>

#include <Audio.h>
#include <Audio_null.h>

#ifdef WITH_AO
#include <Audio_ao.h>
#endif
#ifdef WITH_JACK
#include <Audio_jack.h>
#endif
#ifdef WITH_OPENAL
#include <Audio_openal.h>
#endif


typedef struct Driver_info
{
    char* name;
    Audio* (*create)(void);
} Driver_info;


static Driver_info drivers[] =
{
#ifdef WITH_AO
    { "ao", new_Audio_ao },
#endif
#ifdef WITH_JACK
    { "jack", new_Audio_jack },
#endif
#ifdef WITH_OPENAL
    { "openal", new_Audio_openal },
#endif
    { "null", new_Audio_null },
    { NULL, NULL }
};


Driver_info* get_driver(char* name)
{
    Driver_info* cur = &drivers[0];
    while (cur->name != NULL)
    {
        if (strcmp(name, cur->name) == 0)
        {
            return cur;
        }
        ++cur;
    }
    return NULL;
}


Audio* new_Audio(char* name)
{
    assert(name != NULL);
    Driver_info* driver = get_driver(name);
    if (driver == NULL)
    {
        return NULL;
    }
    assert(driver->create != NULL);
    return driver->create();
}


