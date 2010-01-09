

/*
 * Copyright 2010 Tomi Jylhä-Ollila
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


#define _POSIX_SOURCE
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>

#include <Audio.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>
#include <kunquat/Player_ext.h>
#include <kunquat/Info.h>

#include <keyboard.h>
#include <status_line.h>


#define PLAYER_NAME "kunquat-player"
#define PLAYER_VERSION "0.0.1"


static char* driver_names[] =
{
#if defined(WITH_PULSE)
    "pulse",
#endif
#if defined(WITH_AO)
    "ao",
#endif
#if defined(WITH_JACK)
    "jack",
#endif
#if defined(WITH_OPENAL)
    "openal",
#endif
    "null",
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
    fprintf(stdout, "   -d drv, --driver drv    Use audio driver drv\n");
    fprintf(stdout, "                           Supported drivers:");
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
    fprintf(stdout, "   --buffer-size n         Use audio buffer size of n frames\n");
    fprintf(stdout, "                           Valid range is [64,262144]\n");
    fprintf(stdout, "   --frequency n           Set mixing frequency to n frames/second\n");
    fprintf(stdout, "                           Valid range is [1000,384000]\n");
    fprintf(stdout, "                           (drivers may set additional restrictions)\n");

    fprintf(stdout, "\nPlayback options:\n");
    fprintf(stdout, "   -s sub, --subsong sub   Play the subsong sub\n");
    fprintf(stdout, "                           Valid range is [0,%d] (or 'all')\n",
                                                KQT_SUBSONGS_MAX - 1);
    
    fprintf(stdout, "\nOther options:\n");
    fprintf(stdout, "   -h, --help              Show this help and exit\n");
    fprintf(stdout, "   -q, --quiet             Quiet and non-interactive operation\n");
    fprintf(stdout, "                           (only error messages will be displayed)\n");
    fprintf(stdout, "   --disable-unicode       Don't use Unicode for display\n");
    fprintf(stdout, "   --version               Display version information and exit\n");
    
    fprintf(stdout, "\nSupported keys in interactive mode:\n");
    fprintf(stdout, "   Space                   Pause/resume playback\n");
    fprintf(stdout, "   [0-9], 'a'              Select subsong ('a' plays all subsongs)\n");
    fprintf(stdout, "   Left                    Seek backwards 10 seconds\n");
    fprintf(stdout, "   Right                   Seek forwards 10 seconds\n");
    fprintf(stdout, "   'p'                     Previous subsong\n");
    fprintf(stdout, "   'n'                     Next subsong\n");
    fprintf(stdout, "   Backspace               Previous file\n");
    fprintf(stdout, "   Return                  Next file\n");
    fprintf(stdout, "   'q'                     Quit\n");
    
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
    fprintf(stdout, "Copyright 2010 Tomi Jylhä-Ollila\n");
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
    int start_subsong = -1;
    bool unicode = true;
    long buffer_size = 0;
    long frequency = 0;

    struct option long_options[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "driver", required_argument, NULL, 'd' },
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
                    start_subsong = -1;
                }
                else
                {
                    start_subsong = read_long(optarg, "Subsong", 0, KQT_SUBSONGS_MAX - 1);
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
                fprintf(stderr, "Option %s requires an argument.\n", argv[opt_index]);
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
    bool audio_failed = false;
    if (!audio_failed && buffer_size > 0)
    {
        if (!Audio_set_buffer_size(audio, buffer_size))
        {
            audio_failed = true;
        }
    }
    if (!audio_failed && frequency > 0)
    {
        if (!Audio_set_freq(audio, frequency))
        {
            audio_failed = true;
        }
    }
    if (!audio_failed && !Audio_open(audio))
    {
        audio_failed = true;
    }
    if (audio_failed)
    {
        fprintf(stderr, "%s: %s.\n", Audio_get_name(audio), Audio_get_error(audio));
        del_Audio(audio);
        exit(EXIT_FAILURE);
    }
    if (interactive)
    {
        fprintf(stderr, "Audio driver: %s\n", driver_selection);
    }
    
    if (interactive && !set_terminal(true, true))
    {
        fprintf(stderr, "Couldn't set terminal attributes\n");
    }

    bool quit = false;
    for (int file_arg = optind; file_arg < argc && !quit; ++file_arg)
    {
        struct stat* info = &(struct stat){ .st_mode = 0 };
        errno = 0;
        if (stat(argv[file_arg], info) != 0)
        {
            fprintf(stderr, "Couldn't access information about path %s: %s.\n",
                    argv[file_arg], strerror(errno));
            continue;
        }
        kqt_Handle* handle = NULL;
        if (S_ISDIR(info->st_mode))
        {
            handle = kqt_new_Handle_rw(audio->nframes, argv[file_arg]);
        }
        else
        {
            handle = kqt_new_Handle_r(audio->nframes, argv[file_arg]);
        }
        if (handle == NULL)
        {
            fprintf(stderr, "%s.\n", kqt_Handle_get_error(NULL));
            continue;
        }
        int subsong = start_subsong;
        if (subsong != -1)
        {
            if (!kqt_Handle_set_position(handle, subsong, 0))
            {
                fprintf(stderr, "%s.\n", kqt_Handle_get_error(handle));
                kqt_del_Handle(handle);
                continue;
            }
        }

        Audio_set_handle(audio, handle);

        uint64_t length_ns = kqt_Handle_get_duration(handle);
        uint64_t clipped[2] = { 0 };

        const int status_line_max = 256;
        static char status_line[256] = { '\0' };
        int status_line_chars_used = 0;
        
        Mix_state* mix_state = Mix_state_init(MIX_STATE_AUTO);
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
                                                         length_ns,
                                                         max_voices,
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
                if (key == 'q')
                {
                    quit = true;
                    break;
                }
                else if (key == KEY_LEFT)
                {
                    Audio_pause(audio, true);
                    Audio_get_state(audio, mix_state);
                    long long ns = kqt_Handle_get_position(handle);
                    ns -= 10000000000LL;
                    if (ns < 0)
                    {
                        ns = 0;
                    }
                    if (!kqt_Handle_set_position(handle, subsong, ns))
                    {
                        fprintf(stderr, "\n%s\n", kqt_Handle_get_error(handle));
                    }
                    Audio_pause(audio, false);
                }
                else if (key == KEY_RIGHT)
                {
                    Audio_pause(audio, true);
                    Audio_get_state(audio, mix_state);
                    long long ns = kqt_Handle_get_position(handle);
                    ns += 10000000000LL;
                    if (!kqt_Handle_set_position(handle, subsong, ns))
                    {
                        fprintf(stderr, "\n%s\n", kqt_Handle_get_error(handle));
                    }
                    Audio_pause(audio, false);
                }
                else if ((key >= '0' && key <= '9') || key == 'a'
                         || key == 'p' || key == 'n')
                {
                    int new_subsong = -1;
                    if (key == 'a')
                    {
                        new_subsong = -1;
                    }
                    else if (key == 'p')
                    {
                        new_subsong = subsong - 1;
                    }
                    else if (key == 'n')
                    {
                        new_subsong = subsong + 1;
                    }
                    else
                    {
                        new_subsong = key - '0';
                    }
                    if (new_subsong >= -1 && new_subsong < KQT_SUBSONGS_MAX
                            && kqt_Handle_get_subsong_length(handle, new_subsong) > 0)
                    {
                        subsong = new_subsong;
                        Audio_pause(audio, true);
                        Audio_get_state(audio, mix_state);
                        if (!kqt_Handle_set_position(handle, subsong, 0))
                        {
                            fprintf(stderr, "\n%s\n", kqt_Handle_get_error(handle));
                        }
                        else
                        {
                            length_ns = kqt_Handle_get_duration(handle);
                        }
                        Audio_pause(audio, false);
                    }
                }
                else if (key == KEY_BACKSPACE)
                {
                    if (file_arg > optind)
                    {
                        file_arg -= 2;
                        break;
                    }
                }
                else if (key == KEY_RETURN)
                {
                    if (file_arg < argc - 1)
                    {
                        break;
                    }
                }
            }
            Audio_get_state(audio, mix_state);
        }
        if (interactive)
        {
            fprintf(stderr, "\n");
        }
        Audio_set_handle(audio, NULL);
        Audio_get_state(audio, mix_state);
        kqt_del_Handle(handle);
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


