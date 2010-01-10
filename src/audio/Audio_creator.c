

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <Audio.h>
#include <Audio_null.h>

#ifdef WITH_PULSE
#include <Audio_pulse.h>
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
    char* full_name;
    Audio* (*create)(void);
} Driver_info;


static Driver_info drivers[] =
{
#ifdef WITH_PULSE
    { "pulse", "PulseAudio output", new_Audio_pulse },
#endif
#ifdef WITH_JACK
    { "jack", "JACK output", new_Audio_jack },
#endif
#ifdef WITH_OPENAL
    { "openal", "OpenAL output", new_Audio_openal },
#endif
    { "null", "Null output", new_Audio_null },
    { NULL, NULL, NULL }
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
    Audio* audio = driver->create();
    if (audio == NULL)
    {
        return NULL;
    }
    audio->full_name = driver->full_name;
    return audio;
}


