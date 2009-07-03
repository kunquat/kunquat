

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


#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <getopt.h>

#include <Audio.h>

#include <kqt_Context.h>
#include <kqt_Error.h>
#include <kqt_Reltime.h>


static char* driver_names[] =
{
#if defined(ENABLE_AO)
    "ao",
#endif
#if defined(ENABLE_JACK)
    "jack",
#endif
#if defined(ENABLE_OPENAL)
    "openal",
#endif
    NULL
};


void usage(void)
{
    fprintf(stderr, "Usage: kunquat-player [options] <files>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -h, --help                 Show this help\n");
    fprintf(stderr, "   -d <drv>, --driver=<drv>   Choose audio driver\n");
    fprintf(stderr, "                              Supported drivers:");
    if (driver_names[0] == NULL)
    {
        fprintf(stderr, " (none)\n");
        return;
    }
    fprintf(stderr, " %s", driver_names[0]);
    for (int i = 1; driver_names[i] != NULL; ++i)
    {
        fprintf(stderr, ", %s", driver_names[i]);
    }
    fprintf(stderr, "\n");
    return;
}


void get_minutes_seconds(uint64_t frames, uint32_t freq, int* minutes, double* seconds)
{
    assert(freq > 0);
    assert(minutes != NULL);
    assert(seconds != NULL);
    *minutes = (frames / freq / 60) % 60;
    *seconds = remainder((double)frames / freq, 60);
    if (*seconds < 0)
    {
        *seconds += 60;
    }
    return;
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        usage();
        exit(EXIT_FAILURE);
    }
    char* driver_selection = driver_names[0];
    struct option long_options[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "driver", required_argument, NULL, 'd' },
        { NULL, 0, NULL, 0 }
    };
    int opt = 0;
    int opt_index = 1;
    while ((opt = getopt_long(argc, argv, ":hd:", long_options, &opt_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
            {
                usage();
                exit(EXIT_SUCCESS);
            }
            break;
            case 'd':
            {
                driver_selection = optarg;
            }
            break;
            case ':':
            {
                fprintf(stderr, "Option %s requires an argument\n", argv[opt_index]);
                fprintf(stderr, "Use -h for help.\n");
                exit(EXIT_FAILURE);
            }
            break;
            case '?':
            {
                fprintf(stderr, "Unrecognised option: %s\n", argv[opt_index]);
                fprintf(stderr, "Use -h for help.\n");
                exit(EXIT_FAILURE);
            }
            break;
            default:
            {
                fprintf(stderr, "Use -h for help.\n");
                exit(EXIT_FAILURE);
            }
        }
        opt_index = optind;
    }
    if (optind >= argc)
    {
        fprintf(stderr, "No input files.\n");
        exit(EXIT_SUCCESS);
    }
    Audio* audio = new_Audio(driver_selection);
    if (audio == NULL)
    {
        fprintf(stderr, "Couldn't open the audio driver %s.\n", driver_selection);
        exit(EXIT_FAILURE);
    }
    for (int file_arg = optind; file_arg < argc; ++file_arg)
    {
        kqt_Error* error = KQT_ERROR_AUTO;
        kqt_Context* context = kqt_new_Context_from_path(argv[file_arg],
                                                         audio->nframes,
                                                         256, 32,
                                                         error);
        if (context == NULL)
        {
            fprintf(stderr, "%s\n", error->message);
            exit(EXIT_FAILURE);
        }
        Audio_set_context(audio, context);

        uint32_t freq = Audio_get_freq(audio);
        uint64_t length_frames = kqt_Context_get_length(context, freq);
        int minutes_total = 0;
        double seconds_total = 0;
        get_minutes_seconds(length_frames, freq, &minutes_total, &seconds_total);

        const int status_line_max = 256;
        static char status_line[256] = { '\0' };
        int status_line_bytes_used = 0;
        
        kqt_Context_play_song(context);
        kqt_Mix_state* mix_state = kqt_Mix_state_init(KQT_MIX_STATE_AUTO);
        Audio_get_state(audio, mix_state);
        uint16_t max_voices = 0;
        fprintf(stderr, "Playing %s\n", argv[file_arg]);
        while (mix_state->playing)
        {
            int status_line_pos = 0;
            int minutes = 0;
            double seconds = 0;
            get_minutes_seconds(mix_state->frames, freq, &minutes, &seconds);
            uint64_t frames_left = length_frames - mix_state->frames;
            int minutes_left = 0;
            double seconds_left = 0;
            get_minutes_seconds(frames_left, freq, &minutes_left, &seconds_left);
            if (mix_state->voices > max_voices)
            {
                max_voices = mix_state->voices;
            }
            double pos = kqt_Reltime_get_beats(&mix_state->pos) +
                         ((double)kqt_Reltime_get_rem(&mix_state->pos) / KQT_RELTIME_BEAT);
            int printed = snprintf(status_line + status_line_pos,
                                   status_line_max - status_line_pos,
                                   "Subsong: %02" PRIu16
                                   ", Time: %02d:%04.1f",
                                   mix_state->subsong,
                                   minutes, seconds);
            if (printed >= 0)
            {
                status_line_pos += printed;
                if (status_line_pos >= status_line_max)
                {
                    status_line_pos = status_line_max - 1;
                }
            }
            if (length_frames >= mix_state->frames)
            {
                printed = snprintf(status_line + status_line_pos,
                                   status_line_max - status_line_pos,
                                   " [%02d:%04.1f]"
                                   " of %02d:%04.1f",
                                   minutes_left, seconds_left,
                                   minutes_total, seconds_total);
                if (printed >= 0)
                {
                    status_line_pos += printed;
                    if (status_line_pos >= status_line_max)
                    {
                        status_line_pos = status_line_max - 1;
                    }
                }
            }
            printed = snprintf(status_line + status_line_pos,
                               status_line_max - status_line_pos,
                               ", Position: %02" PRIu16 "/%04.1f"
                               ", Voices: %" PRIu16 " (%" PRIu16 ")",
                               mix_state->order, pos,
                               mix_state->voices, max_voices);
            if (printed >= 0)
            {
                status_line_pos += printed;
                if (status_line_pos >= status_line_max)
                {
                    status_line_pos = status_line_max - 1;
                }
            }
            if (status_line_pos >= status_line_bytes_used)
            {
                status_line_bytes_used = status_line_pos;
            }
            else
            {
                for (int i = status_line_pos; i < status_line_bytes_used; ++i)
                {
                    status_line[i] = ' ';
                }
                status_line[status_line_bytes_used] = '\0';
            }
            fprintf(stderr, "%s\r", status_line);
            Audio_get_state(audio, mix_state);
        }
        fprintf(stderr, "\n");
        Audio_set_context(audio, NULL);
        Audio_get_state(audio, mix_state);
        kqt_del_Context(context);
    }
    fprintf(stderr, "Done.\n");
    del_Audio(audio);
    exit(EXIT_SUCCESS);
}


