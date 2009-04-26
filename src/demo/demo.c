

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kunquat.h>

#include "Audio_ao.h"
#include "Audio_jack.h"
#include "demo_song.h"


typedef struct Driver_info
{
    char* name;
    bool (*open)(Playlist* pl);
    void (*close)(void);
} Driver_info;


static Driver_info drivers[] =
{
#ifdef ENABLE_AO
    { "ao", Audio_ao_open, Audio_ao_close },
#endif
#ifdef ENABLE_JACK
    { "jack", Audio_jack_open, Audio_jack_close },
#endif
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


int main(int argc, char** argv)
{
#ifdef ENABLE_JACK
    char* selection = "jack";
#else
    char* selection = "ao";
#endif
    if (argc > 1)
    {
        if (argc == 3 && strcmp(argv[1], "-d") == 0)
        {
            selection = argv[2];
        }
        else
        {
            fprintf(stderr, "Usage: %s -d <driver>\n", argv[0]);
            fprintf(stderr, "Possible values for driver: ");
            for (int i = 0; drivers[i].name != NULL; ++i)
            {
                if (i > 0)
                {
                    fprintf(stderr, ", ");
                }
                fprintf(stderr, "%s", drivers[i].name);
            }
            fprintf(stderr, "\n");
            exit(EXIT_SUCCESS);
        }
    }
    Driver_info* driver = get_driver(selection);
    if (driver == NULL)
    {
        fprintf(stderr, "Driver \"%s\" is not built in the demo.\n", selection);
        exit(EXIT_FAILURE);
    }
    assert(driver->open != NULL);
    assert(driver->close != NULL);
    Playlist* pl = new_Playlist();
    if (pl == NULL)
    {
        fprintf(stderr, "Couldn't create the Playlist\n");
        exit(EXIT_FAILURE);
    }
    if (!driver->open(pl))
    {
        fprintf(stderr, "Couldn't initialise the audio driver\n");
        del_Playlist(pl);
        exit(EXIT_FAILURE);
    }
    Song* song = demo_song_create();
    if (song == NULL)
    {
        fprintf(stderr, "Couldn't create the demo Song\n");
        driver->close();
        del_Playlist(pl);
        exit(EXIT_FAILURE);
    }
    int32_t song_id = Playlist_ins(pl, song);
    if (song_id == -1)
    {
        fprintf(stderr, "Couldn't add the demo Song into the Playlist\n");
        driver->close();
        del_Playlist(pl);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Playing the demo, Enter to quit...\n");
    Playlist_play_song(pl, song_id);
    getchar();
    Playlist_stop_song(pl, song_id);
    driver->close();
    del_Playlist(pl);
    exit(EXIT_SUCCESS);
}


