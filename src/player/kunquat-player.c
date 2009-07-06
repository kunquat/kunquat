

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
#include <errno.h>

#include <getopt.h>

#include <Audio.h>

#include <kqt_Context.h>
#include <kqt_Error.h>
#include <kqt_Reltime.h>

#include <keyboard.h>
#include <status_line.h>


#define PLAYER_NAME "kunquat-player"
#define PLAYER_VERSION "0.0.1"


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
    "null",
    NULL
};


void usage(void)
{
    fprintf(stdout, "Usage: " PLAYER_NAME " [options] <files>\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "   -h, --help                 Show this help\n");
    fprintf(stdout, "   -d <drv>, --driver=<drv>   Use audio driver <drv>\n");
    fprintf(stdout, "                              Supported drivers:");
    if (driver_names[0] == NULL)
    {
        fprintf(stdout, " (none)\n");
        return;
    }
    fprintf(stdout, " %s", driver_names[0]);
    for (int i = 1; driver_names[i] != NULL; ++i)
    {
        fprintf(stdout, ", %s", driver_names[i]);
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "   -q, --quiet                Quiet and non-interactive operation\n");
    fprintf(stdout, "                              (only error messages will be displayed)\n");
    fprintf(stdout, "   -s, --subsong=<s>          Play the selected subsong\n");
                                                   // FIXME: get bounds from lib
    fprintf(stdout, "                              Valid range is -1..255, -1 means all subsongs\n");
    fprintf(stdout, "       --version              Display version information and exit\n");
    return;
}


char* get_iso_today(void)
{
    static char iso_date[] = "yyyy-mm-dd";
    static char* mons[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    int m = 0;
    for (int i = 0; i < 12; ++i)
    {
        if (strncmp(__DATE__, mons[i], 3) == 0)
        {
            m = i + 1;
            break;
        }
    }
    int d = atoi(__DATE__ + 4);
    int y = atoi(__DATE__ + 7);
    if (m == 0 || d == 0 || y == 0)
    {
        strcpy(iso_date, "unknown");
        return iso_date;
    }
    snprintf(iso_date, 11, "%04d-%02d-%02d", y, m, d);
    return iso_date;
}


void print_licence(void)
{
    fprintf(stdout, "Copyright 2009 Tomi Jylhä-Ollila\n");
    fprintf(stdout, "License GPLv3+: GNU GPL version 3 or later"
                    " <http://gnu.org/licenses/gpl.html>\n");
    fprintf(stdout, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stdout, "There is NO WARRANTY, to the extent permitted by law.\n");
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
    bool interactive = true;
    int subsong = -1;

    struct option long_options[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "driver", required_argument, NULL, 'd' },
        { "quiet", no_argument, NULL, 'q' },
        { "subsong", required_argument, NULL, 's' },
        { "version", no_argument, NULL, 'V' },
        { NULL, 0, NULL, 0 }
    };
    int opt = 0;
    int opt_index = 1;
    while ((opt = getopt_long(argc, argv, ":hd:qs:", long_options, &opt_index)) != -1)
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
            case 'q':
            {
                interactive = false;
            }
            break;
            case 's':
            {
                errno = 0;
                char* first_invalid = NULL;
                long result = strtol(optarg, &first_invalid, 0);
                if (optarg[0] == '\0' || *first_invalid != '\0')
                {
                    fprintf(stderr, "Subsong argument must be an integer\n");
                    fprintf(stderr, "Use -h for help.\n");
                    exit(EXIT_FAILURE);
                }
                int err = errno;
                if (err == ERANGE || result < -1 || result > 255) // FIXME: get bounds from lib
                {
                    fprintf(stderr, "Subsong number is out of range\n");
                    fprintf(stderr, "Use -h for help.\n");
                    exit(EXIT_FAILURE);
                }
                subsong = result;
            }
            break;
            case 'V':
            {
                char* iso_date = get_iso_today();
                fprintf(stdout, PLAYER_NAME " " PLAYER_VERSION
                                " (build date %s)\n",
                                iso_date);
                print_licence();
                exit(EXIT_SUCCESS);
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
        if (interactive)
        {
            fprintf(stderr, "No input files.\n");
        }
        exit(EXIT_SUCCESS);
    }
    
    if (interactive && !set_terminal(true, true))
    {
        fprintf(stderr, "Couldn't set terminal attributes\n");
    }

    Audio* audio = new_Audio(driver_selection);
    if (audio == NULL)
    {
        fprintf(stderr, "Couldn't create the audio driver %s.\n", driver_selection);
        exit(EXIT_FAILURE);
    }
    if (!Audio_open(audio))
    {
        fprintf(stderr, "Couldn't open the audio driver %s.\n", driver_selection);
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    bool quit = false;
    for (int file_arg = optind; file_arg < argc && !quit; ++file_arg)
    {
        kqt_Error* error = KQT_ERROR_AUTO;
        kqt_Context* context = kqt_new_Context_from_path(argv[file_arg],
                                                         audio->nframes,
                                                         256, 32,
                                                         error);
        if (context == NULL)
        {
            fprintf(stderr, "%s\n", error->message);
            continue;
        }
        if (subsong != -1)
        {
            char pos[64] = { '\0' };
            snprintf(pos, 64, "%d", subsong);
            if (!kqt_Context_set_position(context, pos, error))
            {
                fprintf(stderr, "%s\n", error->message);
                kqt_del_Context(context);
                continue;
            }
        }
        Audio_set_context(audio, context);

        uint32_t freq = Audio_get_freq(audio);
        uint64_t length_frames = kqt_Context_get_length(context, freq);
        uint64_t clipped[2] = { 0 };

        const int status_line_max = 256;
        static char status_line[256] = { '\0' };
        int status_line_chars_used = 0;
        
        kqt_Mix_state* mix_state = kqt_Mix_state_init(KQT_MIX_STATE_AUTO);
        Audio_get_state(audio, mix_state);
        uint16_t max_voices = 0;
        if (interactive)
        {
            fprintf(stderr, "Playing %s\n", argv[file_arg]);
        }
        while (mix_state->playing)
        {
            if (interactive)
            {
                for (int i = 0; i < 2; ++i)
                {
                    clipped[i] += mix_state->clipped[i];
                }
                if (mix_state->voices > max_voices)
                {
                    max_voices = mix_state->voices;
                }
                status_line_chars_used = get_status_line(status_line,
                                                         status_line_max,
                                                         mix_state,
                                                         status_line_chars_used,
                                                         clipped,
                                                         length_frames,
                                                         max_voices,
                                                         freq);

                fprintf(stderr, "%s\r", status_line);
             
                int key = get_key();
                if (key == 'q')
                {
                    quit = true;
                    break;
                }
                else if (key == ' ')
                {
                    Audio_pause(audio, true);
                    fprintf(stderr, "[ pause ]\r");
                    set_terminal(true, false);
                    get_key();
                    set_terminal(true, true);
                    Audio_pause(audio, false);
                }
            }
            Audio_get_state(audio, mix_state);
        }
        if (interactive)
        {
            fprintf(stderr, "\n");
        }
        Audio_set_context(audio, NULL);
        Audio_get_state(audio, mix_state);
        kqt_del_Context(context);
    }
    if (interactive)
    {
        fprintf(stderr, "Done.\n");
    }
    del_Audio(audio);
    if (interactive && !set_terminal(false, false))
    {
        fprintf(stderr, "Couldn't reset terminal attributes"
                        " -- you may have to run `reset`.\n");
    }
    return EXIT_SUCCESS;
}


