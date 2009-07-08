

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
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <errno.h>

#include <getopt.h>

#include <Audio.h>

#include <kqt_Context.h>
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
    "wav",
    NULL
};


void usage(void);

char* get_iso_today(void);

void print_version(void);

void print_licence(void);


void usage(void)
{
    print_version();
    fprintf(stdout, "\n");
    fprintf(stdout, "Usage: " PLAYER_NAME " [options] <files>\n");

    fprintf(stdout, "\nOutput options:\n");
    fprintf(stdout, "   -d drv, --driver drv   Use audio driver drv\n");
    fprintf(stdout, "                          Supported drivers:");
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
    fprintf(stdout, "   -f file, --file file   Write the audio data into file\n");
    fprintf(stdout, "   --buffer-size n        Use audio buffer size n\n");
    fprintf(stdout, "                          Valid range is [64,262144]\n");
    fprintf(stdout, "   --frequency x          Set mixing frequency to x frames/second\n");
    fprintf(stdout, "                          Valid range is [1000,384000]\n");
    fprintf(stdout, "                          (drivers may set additional restrictions)\n");

    fprintf(stdout, "\nPlayback options:\n");
    fprintf(stdout, "   -s, --subsong s        Play the subsong s\n");
                                               // FIXME: get bounds from lib
    fprintf(stdout, "                          Valid range is [0,255] (or 'all')\n");
    
    fprintf(stdout, "\nOther options:\n");
    fprintf(stdout, "   -h, --help             Show this help and exit\n");
    fprintf(stdout, "   -q, --quiet            Quiet and non-interactive operation\n");
    fprintf(stdout, "                          (only error messages will be displayed)\n");
    fprintf(stdout, "   --disable-unicode      Don't use Unicode for display\n");
    fprintf(stdout, "   --version              Display version information and exit\n");
    
    fprintf(stdout, "\nSupported keys in interactive mode:\n");
    fprintf(stdout, "   Space                  Pause/unpause\n");
    fprintf(stdout, "   [0-9], 'a'             Select subsong ('a' plays all subsongs)\n");
    fprintf(stdout, "   'q'                    Quit\n");
    
    fprintf(stdout, "\n");
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


void print_version(void)
{
    char* iso_date = get_iso_today();
    fprintf(stdout, PLAYER_NAME " " PLAYER_VERSION
                    " (build date %s)\n",
                    iso_date);
    return;
}


long read_long(char* str, char* desc, long min, long max)
{
    assert(str != NULL);
    assert(desc != NULL);
    assert(min <= max);
    char* first_invalid = NULL;
    long result = strtol(str, &first_invalid, 0);
    int err = errno;
    if (optarg[0] == '\0' || *first_invalid != '\0')
    {
        fprintf(stderr, "%s must be an integer\n", desc);
        fprintf(stderr, "Use -h for help.\n");
        exit(EXIT_FAILURE);
    }
    if (err == ERANGE || result < min || result > max)
    {
        fprintf(stderr, "%s is out of range [%ld,%ld]\n", desc, min, max);
        fprintf(stderr, "Use -h for help.\n");
        exit(EXIT_FAILURE);
    }
    return result;
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
    bool unicode = true;
    long buffer_size = 0;
    long frequency = 0;
    char* out_path = NULL;

    struct option long_options[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "driver", required_argument, NULL, 'd' },
        { "file", required_argument, NULL, 'f' },
        { "buffer-size", required_argument, NULL, 'b' },
        { "frequency", required_argument, NULL, 'F' },
        { "quiet", no_argument, NULL, 'q' },
        { "subsong", required_argument, NULL, 's' },
        { "disable-unicode", no_argument, NULL, 'U' },
        { "version", no_argument, NULL, 'V' },
        { NULL, 0, NULL, 0 }
    };
    int opt = 0;
    int opt_index = 1;
    while ((opt = getopt_long(argc, argv, ":hd:f:qs:", long_options, &opt_index)) != -1)
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
            case 'f':
            {
                out_path = optarg;
            }
            break;
            case 'b':
            {
                buffer_size = read_long(optarg, "Buffer size", 64, 262144);
            }
            break;
            case 'F':
            {
                frequency = read_long(optarg, "Mixing frequency", 1000, 384000);
            }
            break;
            case 'q':
            {
                interactive = false;
            }
            break;
            case 's':
            {
                if (strcmp(optarg, "all") == 0 || strcmp(optarg, "a") == 0)
                {
                    subsong = -1;
                }
                else
                {
                    subsong = read_long(optarg, "Subsong", 0, 255); // FIXME: get bounds from lib
                }
            }
            break;
            case 'U':
            {
                unicode = false;
            }
            break;
            case 'V':
            {
                print_version();
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

    Audio* audio = new_Audio(driver_selection);
    if (audio == NULL)
    {
        fprintf(stderr, "Couldn't create the audio driver %s.\n", driver_selection);
        exit(EXIT_FAILURE);
    }
    if (out_path != NULL)
    {
        if (!Audio_set_file(audio, out_path))
        {
            fprintf(stderr, "%s: %s.\n", Audio_get_name(audio), Audio_get_error(audio));
            del_Audio(audio);
            exit(EXIT_FAILURE);
        }
    }
    if (buffer_size > 0)
    {
        if (!Audio_set_buffer_size(audio, buffer_size))
        {
            fprintf(stderr, "%s: %s.\n", Audio_get_name(audio), Audio_get_error(audio));
            del_Audio(audio);
            exit(EXIT_FAILURE);
        }
    }
    if (frequency > 0)
    {
        if (!Audio_set_freq(audio, frequency))
        {
            fprintf(stderr, "%s: %s.\n", Audio_get_name(audio), Audio_get_error(audio));
            del_Audio(audio);
            exit(EXIT_FAILURE);
        }
    }
    if (!Audio_open(audio))
    {
        fprintf(stderr, "%s: %s.\n", Audio_get_name(audio), Audio_get_error(audio));
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    
    if (interactive && !set_terminal(true, true))
    {
        fprintf(stderr, "Couldn't set terminal attributes\n");
    }

    bool quit = false;
    for (int file_arg = optind; file_arg < argc && !quit; ++file_arg)
    {
        kqt_Context* context = kqt_new_Context(1, audio->nframes, 256, 32);
        if (context == NULL)
        {
            fprintf(stderr, "Couldn't allocate memory for the Kunquat Context\n");
            continue;
        }
        if (!kqt_Context_load(context, argv[file_arg]))
        {
            fprintf(stderr, "%s\n", kqt_Context_get_error(context));
            kqt_del_Context(context);
            continue;
        }
        if (subsong != -1)
        {
            char pos[64] = { '\0' };
            snprintf(pos, 64, "%d", subsong);
            if (!kqt_Context_set_position(context, pos))
            {
                fprintf(stderr, "%s\n", kqt_Context_get_error(context));
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
                                                         freq,
                                                         unicode);

                fprintf(stderr, "\r%s\r", status_line);
             
                int key = get_key();
                if (key == ' ')
                {
                    Audio_pause(audio, true);
                    fprintf(stderr, "\n --- PAUSE ---");
                    set_terminal(true, false);
                    key = get_key();
                    set_terminal(true, true);
                    Audio_pause(audio, false);
                }
                if (tolower(key) == 'q')
                {
                    quit = true;
                    break;
                }
                else if ((key >= '0' && key <= '9') || tolower(key) == 'a')
                {
                    char pos[64] = { '\0' };
                    if (tolower(key) == 'a')
                    {
                        strcpy(pos, "-1");
                    }
                    else
                    {
                        pos[0] = key;
                        pos[1] = '\0';
                    }
                    Audio_pause(audio, true);
                    Audio_get_state(audio, mix_state);
                    if (!kqt_Context_set_position(context, pos))
                    {
                        fprintf(stderr, "%s\n", kqt_Context_get_error(context));
                    }
                    else
                    {
                        length_frames = kqt_Context_get_length(context, freq);
                    }
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


