

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#define _POSIX_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <getopt.h>

#include <sndfile.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>
#include <kunquat/Player_ext.h>


#define PROGRAM_NAME "kunquat-export"
#define PROGRAM_VERSION "0.0.1"


void usage(void);

void print_version(void);

void print_licence(void);

char* create_output_name(const char* input_name, const char* format);


void usage(void)
{
    print_version();
    fprintf(stdout, "\n");
    fprintf(stdout, "Usage: " PROGRAM_NAME " [options] <files>\n");

    fprintf(stdout, "\nOptions:\n");
    fprintf(stdout, "   -o, --output out    Use output file out\n");
    fprintf(stdout, "   -f, --format fmt    Use output file format fmt\n");
    fprintf(stdout, "                       Supported formats: wav, au, flac\n");
    fprintf(stdout, "   -b, --bits b        Use b bits per output frame\n");
    fprintf(stdout, "   --float             Use floating point frames for output\n");
    fprintf(stdout, "   --frequency n       Set mixing frequency to n frames/second\n");
    fprintf(stdout, "                       Valid range is [1000,384000]\n");
    fprintf(stdout, "   -s, --subsong sub   Export subsong sub\n");
    fprintf(stdout, "                       Valid range is [0,255] (or 'all')\n");
    fprintf(stdout, "   -h, --help          Show this help and exit\n");
    fprintf(stdout, "   -q, --quiet         Suppress status messages\n");
    fprintf(stdout, "   --version           Display version information and exit\n");

    fprintf(stdout, "\n");
    return;
}


void print_licence(void)
{
    fprintf(stdout, "Author: Tomi Jylhä-Ollila\n");
    fprintf(stdout, "No rights reserved\n");
    fprintf(stdout, "CC0 1.0 Universal, "
            "http://creativecommons.org/publicdomain/zero/1.0/\n");
    return;
}


void print_version(void)
{
    fprintf(stdout, PROGRAM_NAME " " PROGRAM_VERSION "\n");
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


#define OUT_BUFFER_SIZE (2048)


#define cleanup(handle, buffer, output, explicit_output) \
    if (true)                                            \
    {                                                    \
        if ((handle) != NULL)                            \
        {                                                \
            kqt_del_Handle((handle));                    \
        }                                                \
        if ((out_buf) != NULL)                           \
        {                                                \
            free((out_buf));                             \
        }                                                \
        if (!(explicit_output))                          \
        {                                                \
            assert((output) != NULL);                    \
            free((output));                              \
            (output) = NULL;                             \
        }                                                \
    } else (void)0

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        usage();
        exit(EXIT_FAILURE);
    }

    bool quiet = false;
    char* output = NULL;
    long subsong = -1;
    char* format = NULL;
    long frequency = 48000;
    long bits = 0;
    bool float_frames = false;
    bool explicit_output = false;

    struct option long_options[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "quiet", no_argument, NULL, 'q' },
        { "output", required_argument, NULL, 'o' },
        { "subsong", required_argument, NULL, 's' },
        { "format", required_argument, NULL, 'f' },
        { "bits", required_argument, NULL, 'b' },
        { "float", no_argument, NULL, 'G' },
        { "frequency", required_argument, NULL, 'F' },
        { "version", no_argument, NULL, 'V' },
        { NULL, 0, NULL, 0 }
    };
    int opt = 0;
    int opt_index = 1;
    while ((opt = getopt_long(argc, argv, ":hqo:s:f:b:GF:", long_options, &opt_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
            {
                usage();
                exit(EXIT_SUCCESS);
            }
            break;
            case 'q':
            {
                quiet = true;
            }
            break;
            case 'o':
            {
                output = optarg;
                explicit_output = true;
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
                    subsong = read_long(optarg, "Subsong", 0, KQT_SUBSONGS_MAX - 1);
                }
            }
            break;
            case 'f':
            {
                if (strcmp(optarg, "wav") != 0
                        && strcmp(optarg, "au") != 0
                        && strcmp(optarg, "flac") != 0)
                {
                    fprintf(stderr, "Unsupported format: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                format = optarg;
            }
            break;
            case 'b':
            {
                bits = atoi(optarg);
                if ((bits % 8) != 0 || bits < 8)
                {
                    fprintf(stderr, "Bit depth must be 8, 16, 24 or 32\n");
                    fprintf(stderr, "(32 or 64 for floating point frames)\n");
                    exit(EXIT_FAILURE);
                }
            }
            break;
            case 'G':
            {
                if (bits > 0 && bits < 32)
                {
                    fprintf(stderr, "Floating point frames must be 32 or 64 bits\n");
                }
                float_frames = true;
            }
            break;
            case 'F':
            {
                frequency = read_long(optarg, "Mixing frequency", 1000, 384000);
            }
            break;
            case 'V':
            {
                print_version();
                print_licence();
                exit(EXIT_SUCCESS);
            }
            break;
            case '?':
            {
                fprintf(stderr, "Unrecognised option: %s\n", argv[opt_index]);
            }
            default:
            {
                fprintf(stderr, "Use -h for help.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    if (format == NULL)
    {
        char* extension = NULL;
        if (explicit_output && (extension = strrchr(output, '.')) != NULL)
        {
            if (strcmp(extension, ".wav") == 0)
            {
                format = "wav";
            }
            else if (strcmp(extension, ".au") == 0)
            {
                format = "au";
            }
            else if (strcmp(extension, ".flac") == 0)
            {
                format = "flac";
            }
            else
            {
                format = "wav";
            }
        }
        else
        {
            format = "wav";
        }
    }
    if (bits == 0)
    {
        if (float_frames)
        {
            bits = 32;
        }
        else
        {
            bits = 16;
        }
    }
    else
    {
        if (float_frames)
        {
            if (bits != 32 && bits != 64)
            {
                fprintf(stderr, "Bit depth must be 32 or 64 for floating-point frames\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if (bits < 8 || bits > 32)
            {
                fprintf(stderr, "Bit depth must be 8, 16, 24 or 32 for fixed-point frames\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    if (optind >= argc)
    {
        fprintf(stderr, "No input files.\n");
        exit(EXIT_SUCCESS);
    }
    if (optind < argc - 1 && output != NULL)
    {
        fprintf(stderr, "Output file can be specified with only one input file.\n");
        exit(EXIT_FAILURE);
    }

    SF_INFO sfinfo;
    sfinfo.samplerate = frequency;
    sfinfo.channels = 2;
    sfinfo.format = 0;
    if (strcmp(format, "wav") == 0)
    {
        sfinfo.format = SF_FORMAT_WAV;
    }
    else if (strcmp(format, "au") == 0)
    {
        sfinfo.format = SF_FORMAT_AU;
    }
    else if (strcmp(format, "flac") == 0)
    {
        sfinfo.format = SF_FORMAT_FLAC;
    }
    if (float_frames)
    {
        if (bits == 32)
        {
            sfinfo.format |= SF_FORMAT_FLOAT;
        }
        else
        {
            assert(bits == 64);
            sfinfo.format |= SF_FORMAT_DOUBLE;
        }
    }
    else
    {
        assert(bits <= 32);
        int bit_select[33] = { 0 };
        if (sfinfo.format == SF_FORMAT_WAV)
        {
            bit_select[8] = SF_FORMAT_PCM_U8;
        }
        else
        {
            bit_select[8] = SF_FORMAT_PCM_S8;
        }
        bit_select[16] = SF_FORMAT_PCM_16;
        bit_select[24] = SF_FORMAT_PCM_24;
        bit_select[32] = SF_FORMAT_PCM_32;
        sfinfo.format |= bit_select[bits];
    }
    if (!sf_format_check(&sfinfo))
    {
        fprintf(stderr, "Unsupported format parameter combination: ");
        fprintf(stderr, "%s, %s, %ld bits\n", format,
                float_frames ? "floating point" : "fixed point",
                bits);
        exit(EXIT_FAILURE);
    }

    float* out_buf = malloc(sizeof(float) * OUT_BUFFER_SIZE * sfinfo.channels);
    if (out_buf == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for output buffers.\n");
        exit(EXIT_FAILURE);
    }

    for (int file_arg = optind; file_arg < argc; ++file_arg)
    {
        struct stat* info = &(struct stat){ .st_mode = 0 };
        errno = 0;
        if (stat(argv[file_arg], info) != 0)
        {
            fprintf(stderr, "Coudn't access information about path %s: %s.",
                    argv[file_arg], strerror(errno));
            continue;
        }
        kqt_Handle* handle = NULL;
        if (S_ISDIR(info->st_mode))
        {
            handle = kqt_new_Handle_rw(OUT_BUFFER_SIZE, argv[file_arg]);
        }
        else
        {
            handle = kqt_new_Handle_r(OUT_BUFFER_SIZE, argv[file_arg]);
        }
        if (handle == NULL)
        {
            fprintf(stderr, "%s.\n", kqt_Handle_get_error(NULL));
            continue;
        }
        if (!kqt_Handle_set_position(handle, subsong, 0))
        {
            fprintf(stderr, "%s.\n", kqt_Handle_get_error(handle));
            kqt_del_Handle(handle);
            continue;
        }

        if (output == NULL)
        {
            assert(!explicit_output);
            output = create_output_name(argv[file_arg], format);
            if (output == NULL)
            {
                cleanup(handle, out_buf, output, explicit_output);
                fprintf(stderr, "Couldn't allocate memory for output file name.\n");
                exit(EXIT_FAILURE);
            }
        }
        SNDFILE* out = sf_open(output, SFM_WRITE, &sfinfo);
        if (out == NULL)
        {
            fprintf(stderr, "Couldn't create file %s: %s\n", output, sf_strerror(NULL));
            cleanup(handle, out_buf, output, explicit_output);
            exit(EXIT_FAILURE);
        }
        long long duration = kqt_Handle_get_duration(handle);
        long mixed = 0;
        long long total = 0;
        while ((mixed = kqt_Handle_mix(handle, OUT_BUFFER_SIZE, frequency)) > 0)
        {
            kqt_frame* buf_l = kqt_Handle_get_buffer(handle, 0);
            kqt_frame* buf_r = kqt_Handle_get_buffer(handle, 1);
            if (buf_r == NULL)
            {
                buf_r = buf_l;
            }
            for (long i = 0; i < mixed; ++i)
            {
                out_buf[i * 2] = (float)buf_l[i];
                out_buf[(i * 2) + 1] = (float)buf_r[i];
            }
            sf_writef_float(out, out_buf, mixed);
            total += mixed;
            long long pos = kqt_Handle_get_position(handle);
            if (!quiet)
            {
                fprintf(stderr, "%4.1f %%", ((double)pos / duration) * 100);
                long long clipped = kqt_Handle_get_clipped(handle, 0);
                clipped += kqt_Handle_get_clipped(handle, 1);
                if (clipped > 0)
                {
                    fprintf(stderr, ", clipped: %lld", clipped);
                }
                fprintf(stderr, "\r");
            }
        }
        int err = sf_close(out);
        if (err != 0)
        {
            fprintf(stderr, "Couldn't close the output file: %s\n", output);
        }
        else if (!quiet)
        {
            double amps[4] = { 0 };
            amps[0] = -kqt_Handle_get_min_amplitude(handle, 0);
            amps[1] = -kqt_Handle_get_min_amplitude(handle, 1);
            amps[2] = kqt_Handle_get_max_amplitude(handle, 0);
            amps[3] = kqt_Handle_get_max_amplitude(handle, 1);
            double max_amp = 0;
            for (int i = 0; i < 4; ++i)
            {
                if (max_amp < amps[i])
                {
                    max_amp = amps[i];
                }
            }
            max_amp = log2(max_amp) * 6;
            fprintf(stderr, "Wrote %lld frames into %s, "
                            "peak amplitude: %+.2f dBFS",
                            total, output, max_amp);
            long long clipped = kqt_Handle_get_clipped(handle, 0);
            clipped += kqt_Handle_get_clipped(handle, 1);
            if (clipped > 0)
            {
                fprintf(stderr, ", clipped: %lld items", clipped);
            }
            fprintf(stderr, "\n");
        }

        kqt_del_Handle(handle);
        if (!explicit_output)
        {
            free(output);
            output = NULL;
        }
    }
    free(out_buf);
    exit(EXIT_SUCCESS);
}

#undef cleanup


char* create_output_name(const char* input_name, const char* format)
{
    assert(input_name != NULL);
    char* last_slash = strrchr(input_name, '/');
    bool ends_in_slash = last_slash != NULL &&
                         last_slash == &input_name[strlen(input_name) - 1];
    const char* ext_pos = strstr(input_name, ".kqt");
    const char* last_ext_pos = ext_pos;
    while (last_ext_pos != NULL)
    {
        last_ext_pos = strstr(ext_pos + 1, ".kqt");
        if (last_ext_pos != NULL)
        {
            ext_pos = last_ext_pos;
        }
    }
    if (ext_pos != NULL && last_slash != NULL && ext_pos < last_slash)
    {
        ext_pos = NULL;
    }
    if (ext_pos == NULL)
    {
        int new_ext_pos = strlen(input_name);
        if (ends_in_slash)
        {
            int last_non_slash = last_slash - input_name;
            while (last_non_slash >= 0 && input_name[last_non_slash] == '/')
            {
                --last_non_slash;
            }
            new_ext_pos = last_non_slash + 1;
        }
        ext_pos = &input_name[new_ext_pos];
    }
    assert(ext_pos != NULL);
    assert(ext_pos >= input_name);
    char* output_name = calloc(ext_pos + strlen(format) + 2 - input_name, sizeof(char));
    if (output_name == NULL)
    {
        return NULL;
    }
    strncpy(output_name, input_name, ext_pos - input_name);
    strcat(output_name, ".");
    strcat(output_name, format);
    return output_name;
}


