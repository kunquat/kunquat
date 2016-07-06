

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Sample_params.h>

#include <debug/assert.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


Sample_params* Sample_params_init(Sample_params* params)
{
    rassert(params != NULL);

    params->format = SAMPLE_FORMAT_WAVPACK;
    params->mid_freq = 48000;
    params->loop = SAMPLE_LOOP_OFF;
    params->loop_start = 0;
    params->loop_end = 0;

    return params;
}


static bool read_field(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

    Sample_params* params = userdata;

    if (string_eq(key, "format"))
    {
        char format[32] = "";
        if (!Streader_read_string(sr, 32, format))
            return false;

        if (string_eq(format, "WavPack"))
        {
            params->format = SAMPLE_FORMAT_WAVPACK;
        }
        /*else if (string_eq(format, "Ogg Vorbis"))
        {
            params->format = SAMPLE_FORMAT_VORBIS;
        } */
        else
        {
            Streader_set_error(sr, "Unrecognised Sample format: %s", format);
            return false;
        }
    }
    else if (string_eq(key, "freq"))
    {
        if (!Streader_read_float(sr, &params->mid_freq))
            return false;
        if (!(params->mid_freq > 0))
        {
            Streader_set_error(sr, "Sample frequency is not positive");
            return false;
        }
    }
    else if (string_eq(key, "loop_mode"))
    {
        char mode[5] = "off!";
        if (!Streader_read_string(sr, 5, mode))
            return false;

        if (string_eq(mode, "off"))
        {
            params->loop = SAMPLE_LOOP_OFF;
        }
        else if (string_eq(mode, "uni"))
        {
            params->loop = SAMPLE_LOOP_UNI;
        }
        else if (string_eq(mode, "bi"))
        {
            params->loop = SAMPLE_LOOP_BI;
        }
        else
        {
            Streader_set_error(
                    sr,
                    "Invalid Sample loop mode (must be"
                        " \"off\", \"uni\" or \"bi\")");
            return false;
        }
    }
    else if (string_eq(key, "loop_start"))
    {
        int64_t loop_start = 0;
        if (!Streader_read_int(sr, &loop_start))
            return false;

        if (loop_start < 0)
        {
            Streader_set_error(sr, "Sample loop start must be non-negative");
            return false;
        }

        params->loop_start = loop_start;
    }
    else if (string_eq(key, "loop_end"))
    {
        int64_t loop_end = 0;
        if (!Streader_read_int(sr, &loop_end))
            return false;

        if (loop_end < 0)
        {
            Streader_set_error(sr, "Sample loop end must be non-negative");
            return false;
        }

        params->loop_end = loop_end;
    }
    else
    {
        Streader_set_error(sr, "Unsupported key in Sample header: %s", key);
        return false;
    }

    return true;
}

bool Sample_params_parse(Sample_params* params, Streader* sr)
{
    rassert(params != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Sample_params* input = Sample_params_init(
            &(Sample_params){ .format = SAMPLE_FORMAT_NONE });
    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_field, input))
            return false;
    }

    Sample_params_copy(params, input);

    return true;
}


Sample_params* Sample_params_copy(Sample_params* dest, const Sample_params* src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    memcpy(dest, src, sizeof(Sample_params));
    return dest;
}


