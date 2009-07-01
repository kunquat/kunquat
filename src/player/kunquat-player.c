

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
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <Audio.h>
#ifdef ENABLE_AO
#include <Audio_ao.h>
#endif
#ifdef ENABLE_JACK
#include <Audio_jack.h>
#endif
#ifdef ENABLE_OPENAL
#include <Audio_openal.h>
#endif

#include <kunquat.h>
#include <File_tree.h>


typedef struct Driver_info
{
    char* name;
    Audio* (*create)(void);
} Driver_info;


static Driver_info drivers[] =
{
#ifdef ENABLE_AO
    { "ao", new_Audio_ao },
#endif
#ifdef ENABLE_JACK
    { "jack", new_Audio_jack },
#endif
#ifdef ENABLE_OPENAL
    { "openal", new_Audio_openal },
#endif
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


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <kunquat_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* driver_selection = NULL;
#if defined(ENABLE_AO)
    driver_selection = "ao";
#elif defined(ENABLE_JACK)
    driver_selection = "jack";
#elif defined(ENABLE_OPENAL)
    driver_selection = "openal";
#else
#error "kunquat-player requires an audio driver but none were configured."
#endif
    if (argc >= 4 && strcmp(argv[2], "-d") == 0)
    {
        driver_selection = argv[3];
    }
    Driver_info* driver = get_driver(driver_selection);
    if (driver == NULL)
    {
        fprintf(stderr, "Unsupported driver: %s\n", driver_selection);
        exit(EXIT_FAILURE);
    }
    assert(driver->create != NULL);
    Audio* audio = driver->create();
    if (audio == NULL)
    {
        fprintf(stderr, "Couldn't open the audio driver.\n");
        exit(EXIT_FAILURE);
    }
    Song* song = new_Song(2, audio->nframes, 16);
    if (song == NULL)
    {
        fprintf(stderr, "Couldn't create the Song\n");
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    Read_state* state = READ_STATE_AUTO;
    File_tree* tree = new_File_tree_from_tar(argv[1], state);
    if (tree == NULL)
    {
        fprintf(stderr, "%s:%s:%d: %s\n", argv[1], state->path, state->row, state->message);
        del_Song(song);
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    if (!Song_read(song, tree, state))
    {
        fprintf(stderr, "%s:%d: %s\n", state->path, state->row, state->message);
        del_File_tree(tree);
        del_Song(song);
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    del_File_tree(tree);
    Player* player = new_Player(audio->freq, 256, song);
    if (player == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for Player\n");
        del_Song(song);
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    Audio_set_player(audio, player);
    Player_play_song(player);
    kqt_Mix_state* mix_state = kqt_Mix_state_init(&(kqt_Mix_state){ .playing = true });
    Audio_get_state(audio, mix_state);
    uint16_t max_voices = 0;
    while (mix_state->playing)
    {
        int minutes = (mix_state->frames / Audio_get_freq(audio) / 60) % 60;
        int ints = (mix_state->frames / Audio_get_freq(audio) / 60) % 60;
        double seconds = ((double)mix_state->frames / Audio_get_freq(audio)) - ints;
        if (mix_state->voices > max_voices)
        {
            max_voices = mix_state->voices;
        }
        double pos = kqt_Reltime_get_beats(&mix_state->pos) +
                     ((double)kqt_Reltime_get_rem(&mix_state->pos) / KQT_RELTIME_BEAT);
        fprintf(stderr, "Playing subsong: %02" PRIu16
                        ", time: %02d:%04.1f"
                        ", position: %02" PRIu16 "/%04.1f"
                        ", voices: %" PRIu16 " (%" PRIu16 ")      \r",
                        mix_state->subsong,
                        minutes, seconds,
                        mix_state->order, pos,
                        mix_state->voices, max_voices);
        Audio_get_state(audio, mix_state);
    }
    fprintf(stderr, "\nDone.\n");
    del_Audio(audio);
    del_Player(player);
    exit(EXIT_SUCCESS);
}


