

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

#include <kqt_Context.h>
#include <kqt_Error.h>
#include <kqt_Reltime.h>


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
    Audio* audio = new_Audio(driver_selection);
    if (audio == NULL)
    {
        fprintf(stderr, "Couldn't open the audio driver %s.\n", driver_selection);
        exit(EXIT_FAILURE);
    }
    kqt_Error* error = KQT_ERROR_AUTO;
    kqt_Context* context = kqt_new_Context_from_path(argv[1], audio->nframes, 256, 32, error);
    if (context == NULL)
    {
        fprintf(stderr, "%s\n", error->message);
        exit(EXIT_FAILURE);
    }
    Audio_set_context(audio, context);
    kqt_Context_play_song(context);
    kqt_Mix_state* mix_state = kqt_Mix_state_init(&(kqt_Mix_state){ .playing = false });
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
    kqt_del_Context(context);
    exit(EXIT_SUCCESS);
}


