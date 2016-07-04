

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Wav.h>

#include <debug/assert.h>
#include <init/devices/param_types/Sample.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef WITH_SNDFILE

bool Sample_parse_wav(Sample* sample, Streader* sr)
{
    assert(sample != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_set_error(sr, "This build of libkunquat does not support WAV.");

    return false;
}

#else


#include <sndfile.h>


typedef struct String_context
{
    const char* data;
    sf_count_t length;
    sf_count_t pos;
} String_context;


static sf_count_t get_filelen_str(void* user_data)
{
    assert(user_data != NULL);
    const String_context* context = user_data;
    return context->length;
}


static sf_count_t seek_str(sf_count_t offset, int whence, void* user_data)
{
    assert(user_data != NULL);
    String_context* context = user_data;

    switch (whence)
    {
        case SEEK_SET: context->pos = offset; break;
        case SEEK_CUR: context->pos += offset; break;
        case SEEK_END: context->pos = context->length + offset; break;
    }

    assert(context->pos >= 0);
    assert(context->pos <= context->length);

    return context->pos;
}


static sf_count_t read_str(void* ptr, sf_count_t count, void* user_data)
{
    assert(user_data != NULL);
    String_context* context = user_data;
    const sf_count_t actual_count = min(count, context->length - context->pos);
    memcpy(ptr, &context->data[context->pos], (size_t)actual_count);
    context->pos += actual_count;
    return actual_count;
}


static sf_count_t tell_str(void* user_data)
{
    assert(user_data != NULL);
    const String_context* context = user_data;
    return context->pos;
}


static void close_sndfile(SNDFILE* sf)
{
    const int error = sf_close(sf);
    if (error != SF_ERR_NO_ERROR)
        fprintf(stderr,
                "Warning: libsndfile error on handle close: %s\n",
                sf_strerror(NULL));
    return;
}


bool Sample_parse_wav(Sample* sample, Streader* sr)
{
    assert(sample != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    // Prepare access from memory
    SF_VIRTUAL_IO* sfvirtual = &(SF_VIRTUAL_IO)
    {
        .get_filelen = get_filelen_str,
        .seek        = seek_str,
        .read        = read_str,
        .write       = NULL,
        .tell        = tell_str,
    };

    SF_INFO* sfinfo = &(SF_INFO)
    {
        .frames     = 0,
        .samplerate = 0,
        .channels   = 0,
        .format     = 0,
        .sections   = 0,
        .seekable   = 0,
    };

    String_context* context = &(String_context)
    {
        .data   = sr->str,
        .length = (sf_count_t)sr->len,
        .pos    = 0,
    };

    SNDFILE* sf = sf_open_virtual(sfvirtual, SFM_READ, sfinfo, context);
    if (sf == NULL)
    {
        Streader_set_error(sr, "libsndfile error: %s", sf_strerror(NULL));
        return false;
    }

    // Check format
    if ((sfinfo->format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV)
    {
        Streader_set_error(sr, "Data is not a WAV file");
        close_sndfile(sf);
        return false;
    }

    if ((sfinfo->channels != 1) && (sfinfo->channels != 2))
    {
        Streader_set_error(
                sr, "Unsupported amount of channels (%d) in the data", sfinfo->channels);
        close_sndfile(sf);
        return false;
    }

    // Initialise the sample fields
    sample->channels = sfinfo->channels;
    sample->bits = 32;
    sample->is_float = true;
    sample->len = sfinfo->frames;
    sample->data[0] = sample->data[1] = NULL;

    float* nbuf_l = memory_alloc_items(float, sample->len * (int)sizeof(float));
    if (nbuf_l == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for sample");
        close_sndfile(sf);
        return false;
    }

    float* nbuf_r = NULL;
    if (sample->channels == 2)
    {
        nbuf_r = memory_alloc_items(float, sample->len * (int)sizeof(float));
        if (nbuf_r == NULL)
        {
            memory_free(nbuf_l);
            Streader_set_memory_error(sr, "Could not allocate memory for sample");
            close_sndfile(sf);
            return false;
        }
        sample->data[1] = nbuf_r;
    }

    sample->data[0] = nbuf_l;

    // Read data
    float read_buf[256] = { 0.0f };
    const int read_frames_max = 256 / sample->channels;

    int64_t read_total = 0;
    sf_count_t read_count = sf_readf_float(sf, read_buf, read_frames_max);
    while (read_count > 0)
    {
        const float* left_start = &read_buf[0];
        for (sf_count_t i = 0; i < read_count; ++i)
            nbuf_l[read_total + i] = left_start[i * sample->channels];

        if (sample->channels == 2)
        {
            const float* right_start = &read_buf[1];
            for (sf_count_t i = 0; i < read_count; ++i)
                nbuf_r[read_total + i] = right_start[i * sample->channels];
        }

        read_total += read_count;
        read_count = sf_readf_float(sf, read_buf, read_frames_max);
    }

    // Finish
    close_sndfile(sf);

    return true;
}


#endif // WITH_SNDFILE


