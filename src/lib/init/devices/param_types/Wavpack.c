

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Wavpack.h>

#include <debug/assert.h>
#include <init/Background_loader.h>
#include <init/devices/param_types/Sample.h>
#include <mathnum/common.h>
#include <memory.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef WITH_WAVPACK

bool Sample_parse_wavpack(Sample* sample, Streader* sr, Background_loader* bkg_loader)
{
    rassert(sample != NULL);
    rassert(sr != NULL);
    rassert(bkg_loader != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Streader_set_error(sr, "This build of libkunquat does not support WavPack");

    return false;
}

#else // WITH_WAVPACK


#include <wavpack/wavpack.h>


typedef struct String_context
{
    const char* data;
    int64_t length;
    int64_t pos;
    int push_back;
} String_context;


static int32_t read_bytes_str(void* id, void* data, int32_t bcount)
{
    rassert(id != NULL);
    rassert(data != NULL);

    String_context* sc = id;
    if (sc->pos >= sc->length)
        return 0;

    int64_t left = sc->length - sc->pos;
    int64_t bytes_read = min(left, bcount);
    if (sc->push_back != EOF)
    {
        *((char*)data) = (char)sc->push_back;
        if (bytes_read > 0)
            memcpy((char*)data + 1, sc->data + sc->pos + 1, (uint32_t)bytes_read - 1);
        sc->push_back = EOF;
    }
    else
    {
        memcpy(data, sc->data + sc->pos, (size_t)bytes_read);
    }
#if 0
    if (true || sc->pos < 20)
    {
//        fprintf(stderr, "foo\n");
        for (int i = 0; i < bytes_read; ++i)
        {
//            fprintf(stderr, "%d: %c\n", sc->pos + i, ((char*)data)[i]);
        }
    }
#endif

    sc->pos += bytes_read;

    return (int32_t)bytes_read;
}


static uint32_t get_pos_str(void* id)
{
    rassert(id != NULL);
    const String_context* sc = id;
    return (uint32_t)sc->pos;
}


static int set_pos_abs_str(void* id, uint32_t pos)
{
    rassert(id != NULL);

    String_context* sc = id;
    if (pos > sc->length)
        sc->pos = sc->length;
    else
        sc->pos = pos;

    sc->push_back = EOF;

    return 0;
}


static int set_pos_rel_str(void* id, int32_t delta, int mode)
{
    rassert(id != NULL);

    String_context* sc = id;
    int64_t ref = 0;

    if (mode == SEEK_SET)
        ref = 0;
    else if (mode == SEEK_CUR)
        ref = sc->pos;
    else if (mode == SEEK_END)
        ref = sc->length;
    else
        return -1;

    sc->push_back = EOF;
    ref += delta;
    if (ref < 0)
        ref = 0;
    else if (ref > sc->length)
        ref = sc->length;

    sc->pos = ref;

    return 0;
}


static int push_back_byte_str(void* id, int c)
{
//    fprintf(stderr, "push back %c\n", (char)c);
    rassert(id != NULL);

    String_context* sc = id;
    if (sc->push_back != EOF)
        return EOF;

    if (sc->pos == 0)
        return EOF;

    --sc->pos;
    sc->push_back = c;

    return c;
}


static uint32_t get_length_str(void* id)
{
    rassert(id != NULL);
    const String_context* sc = id;
    return (uint32_t)sc->length;
}


static int can_seek_str(void* id)
{
    rassert(id != NULL);
    return 1;
}


static int32_t write_bytes_str(void* id, void* data, int32_t bcount)
{
    ignore(id);
    ignore(data);
    ignore(bcount);
    rassert(false);
    return 0;
}


static WavpackStreamReader reader_str =
{
    read_bytes_str,
    get_pos_str,
    set_pos_abs_str,
    set_pos_rel_str,
    push_back_byte_str,
    get_length_str,
    can_seek_str,
    write_bytes_str
};


typedef struct Callback_data
{
    int mode;
    int channels;
    int bits;
    int bytes;
    uint32_t len;

    char err_str[80];
    WavpackContext* context;
    void* copied_data;
    Sample* sample;
    String_context sc;
} Callback_data;


static Callback_data* new_Callback_data(void)
{
    Callback_data* cb_data = memory_alloc_item(Callback_data);
    if (cb_data == NULL)
        return NULL;

    cb_data->mode = 0;
    cb_data->channels = 0;
    cb_data->bits = 0;
    cb_data->bytes = 0;
    cb_data->len = 0;

    memset(cb_data->err_str, 0, 80);
    cb_data->context = NULL;
    cb_data->copied_data = NULL;
    cb_data->sample = NULL;

    return cb_data;
}


static void del_Callback_data(Callback_data* cb_data)
{
    if (cb_data == NULL)
        return;

    if (cb_data->context != NULL)
        WavpackCloseFile(cb_data->context);

    memory_free(cb_data->copied_data);
    memory_free(cb_data);

    return;
}


#define read_wp_samples(type, sample, src, count, offset, lshift)       \
    if (true)                                                           \
    {                                                                   \
        type* sample_bufs[] = { sample->data[0], sample->data[1] };     \
                                                                        \
        for (int ch = 0; ch < sample->channels; ++ch)                   \
        {                                                               \
            for (int64_t i = 0; i < count; ++i)                         \
                sample_bufs[ch][offset + i] =                           \
                    (type)(src[i * sample->channels + ch] << lshift);   \
        }                                                               \
    } else ignore(0)

static void load_wavpack_data(Error* error, void* user_data)
{
    rassert(error != NULL);
    rassert(user_data != NULL);

    Callback_data* cb_data = user_data;
    rassert(cb_data->context != NULL);
    rassert(cb_data->sample != NULL);
    rassert(cb_data->sc.data != NULL);

    Sample* sample = cb_data->sample;

    const int req_bytes = sample->bits / 8;

#define WAVPACK_BUFFER_SIZE 256

    int32_t buf[WAVPACK_BUFFER_SIZE] = { 0 };
    int64_t read = WavpackUnpackSamples(
            cb_data->context, buf, (uint32_t)(WAVPACK_BUFFER_SIZE / sample->channels));
    int64_t written = 0;
    while (read > 0 && written < sample->len)
    {
        if (req_bytes == 1)
        {
            read_wp_samples(int8_t, sample, buf, read, written, 0);
        }
        else if (req_bytes == 2)
        {
            read_wp_samples(int16_t, sample, buf, read, written, 0);
        }
        else
        {
            rassert(cb_data->bytes == 4);

            if (sample->is_float)
            {
                float* sample_bufs[] = { sample->data[0], sample->data[1] };
                float* buf_float = (float*)buf;

                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    for (int64_t i = 0; i < read; ++i)
                        sample_bufs[ch][written + i] =
                            buf_float[i * sample->channels + ch];
                }
            }
            else
            {
                const int shift = (cb_data->bits == 24) ? 8 : 0;
                read_wp_samples(int32_t, sample, buf, read, written, shift);
            }
        }

        written += read;
        read = WavpackUnpackSamples(
                cb_data->context,
                buf,
                (uint32_t)(WAVPACK_BUFFER_SIZE / sample->channels));
    }

#undef WAVPACK_BUFFER_SIZE

    if (written < sample->len)
    {
        Error_set_desc(
                error,
                ERROR_RESOURCE,
                __FILE__,
                __LINE__,
                __func__,
                "Couldn't read all sample data");
    }

    return;
}

#undef read_wp_samples


static void cleanup_loader(Error* error, void* user_data)
{
    rassert(error != NULL);
    rassert(user_data != NULL);

    Callback_data* cb_data = user_data;
    rassert(cb_data->context != NULL);
    rassert(cb_data->sample != NULL);
    rassert(cb_data->sc.data != NULL);

    Sample* sample = cb_data->sample;

    del_Callback_data(cb_data);

    if (Error_is_set(error))
    {
        memory_free(sample->data[0]);
        memory_free(sample->data[1]);
        sample->data[0] = sample->data[1] = NULL;
        sample->len = 0;
    }

    return;
}


bool Sample_parse_wavpack(Sample* sample, Streader* sr, Background_loader* bkg_loader)
{
    rassert(sample != NULL);
    rassert(sr != NULL);
    rassert(bkg_loader != NULL);

    if (Streader_is_error_set(sr))
        return false;

    const void* data = sr->str;
    const int64_t length = sr->len;

    Callback_data* cb_data = new_Callback_data();
    if (cb_data == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for WavPack loader");
        return false;
    }

    void* copied_data = NULL;

    if (Background_loader_get_thread_count(bkg_loader) > 0)
    {
        // Try to copy the compressed data for background process
        // (the original might get freed before we finish)
        copied_data = memory_alloc_items(char, length);
        if (copied_data != NULL)
        {
            memcpy(copied_data, data, (size_t)length);
            cb_data->copied_data = copied_data;
        }
    }

    cb_data->sc = (String_context)
    {
        .data = (copied_data != NULL) ? copied_data : data,
        .length = length,
        .pos = 0,
        .push_back = EOF,
    };

    cb_data->context = WavpackOpenFileInputEx(
            &reader_str,
            &cb_data->sc,
            NULL,
            cb_data->err_str,
            OPEN_2CH_MAX | OPEN_NORMALIZE,
            0);
    if (cb_data->context == NULL)
    {
        Streader_set_error(sr, cb_data->err_str);
        del_Callback_data(cb_data);
        return false;
    }

    cb_data->mode = WavpackGetMode(cb_data->context);
    cb_data->channels = WavpackGetReducedChannels(cb_data->context);
    cb_data->bits = WavpackGetBitsPerSample(cb_data->context);
    cb_data->bytes = WavpackGetBytesPerSample(cb_data->context);
    cb_data->len = WavpackGetNumSamples(cb_data->context);

    if (cb_data->len == (uint32_t)-1)
    {
        del_Callback_data(cb_data);
        Streader_set_error(sr, "Couldn't determine WavPack file length");
        return false;
    }

//    sample->params.format = SAMPLE_FORMAT_WAVPACK;
    sample->len = cb_data->len;
    sample->channels = cb_data->channels;
//    sample->params.mid_freq = freq;
//    Sample_set_loop(sample, SAMPLE_LOOP_OFF);
//    Sample_set_loop_start(sample, 0);
//    Sample_set_loop_end(sample, 0);

    if (cb_data->bits <= 8)
        sample->bits = 8;
    else if (cb_data->bits <= 16)
        sample->bits = 16;
    else if (cb_data->bits <= 24)
        sample->bits = 32; // output values will be shifted
    else
        sample->bits = 32;

    if ((cb_data->mode & MODE_FLOAT))
    {
        sample->is_float = true;
        sample->bits = 32;
    }

    const int req_bytes = sample->bits / 8;
    sample->data[0] = sample->data[1] = NULL;
    void* nbuf_l = memory_alloc_items(char, sample->len * req_bytes);
    if (nbuf_l == NULL)
    {
        del_Callback_data(cb_data);
        Streader_set_memory_error(sr, "Could not allocate memory for sample");
        return false;
    }

    if (cb_data->channels == 2)
    {
        void* nbuf_r = memory_alloc_items(char, sample->len * req_bytes);
        if (nbuf_r == NULL)
        {
            memory_free(nbuf_l);
            del_Callback_data(cb_data);

            Streader_set_memory_error(
                    sr, "Could not allocate memory for sample");
            return false;
        }
        sample->data[1] = nbuf_r;
    }

    sample->data[0] = nbuf_l;

    cb_data->sample = sample;

    Background_loader_task* task =
        MAKE_BACKGROUND_LOADER_TASK(load_wavpack_data, cleanup_loader, cb_data);

    if (!Background_loader_add_task(bkg_loader, task))
    {
        Error* error = ERROR_AUTO;
        load_wavpack_data(error, cb_data);
        cleanup_loader(error, cb_data);

        if (Error_is_set(error))
        {
            Error_copy(&sr->error, error);
            return false;
        }
    }

    return true;
}


#endif // WITH_WAVPACK


