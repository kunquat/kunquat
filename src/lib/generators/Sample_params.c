

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <Sample_params.h>
#include <string_common.h>
#include <xassert.h>


Sample_params* Sample_params_init(Sample_params* params)
{
    assert(params != NULL);
    params->format = SAMPLE_FORMAT_NONE;
    params->mid_freq = 48000;
    params->loop = SAMPLE_LOOP_OFF;
    params->loop_start = 0;
    params->loop_end = 0;
    return params;
}


bool Sample_params_parse(Sample_params* params,
                         char* str,
                         Read_state* state)
{
    assert(params != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Sample_params* input = Sample_params_init(
            &(Sample_params){ .format = SAMPLE_FORMAT_NONE });
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            bool expect_key = true;
            while (expect_key)
            {
                char key[128] = { '\0' };
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (string_eq(key, "format"))
                {
                    char format[32] = { '\0' };
                    str = read_string(str, format, 32, state);
                    if (!state->error)
                    {
                        if (string_eq(format, "WavPack"))
                        {
                            input->format = SAMPLE_FORMAT_WAVPACK;
                        }
/*                        else if (string_eq(format, "Ogg Vorbis"))
                        {
                            input->format = SAMPLE_FORMAT_VORBIS;
                        } */
                        else
                        {
                            Read_state_set_error(state,
                                    "Unrecognised Sample format: %s", format);
                        }
                    }
                }
                else if (string_eq(key, "freq"))
                {
                    str = read_double(str, &input->mid_freq, state);
                    if (!(input->mid_freq > 0))
                    {
                        Read_state_set_error(state,
                                "Sample frequency is not positive");
                    }
                }
                else if (string_eq(key, "loop_mode"))
                {
                    char mode[] = "off!";
                    str = read_string(str, mode, 5, state);
                    if (string_eq(mode, "off"))
                    {
                        input->loop = SAMPLE_LOOP_OFF;
                    }
                    else if (string_eq(mode, "uni"))
                    {
                        input->loop = SAMPLE_LOOP_UNI;
                    }
                    else if (string_eq(mode, "bi"))
                    {
                        input->loop = SAMPLE_LOOP_BI;
                    }
                    else
                    {
                        Read_state_set_error(state,
                                "Invalid Sample loop mode (must be"
                                " \"off\", \"uni\" or \"bi\")");
                    }
                }
                else if (string_eq(key, "loop_start"))
                {
                    int64_t loop_start = 0;
                    str = read_int(str, &loop_start, state);
                    input->loop_start = loop_start;
                }
                else if (string_eq(key, "loop_end"))
                {
                    int64_t loop_end = 0;
                    str = read_int(str, &loop_end, state);
                    input->loop_end = loop_end;
                }
                else
                {
                    Read_state_set_error(state,
                            "Unsupported key in Sample header: %s", key);
                }
                if (state->error)
                {
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    Sample_params_copy(params, input);
    return true;
}


Sample_params* Sample_params_copy(Sample_params* dest, Sample_params* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(Sample_params));
    return dest;
}


