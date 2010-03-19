

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <Sample.h>
#include <File_wavpack.h>
#include <Directory.h>
#include <Handle_private.h>
#include <math_common.h>

#include <xmemory.h>


#ifndef WITH_WAVPACK

bool Sample_parse_wavpack(Sample* sample,
                          void* data,
                          long length,
                          Read_state* state)
{
    assert(sample != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    (void)sample;
    (void)data;
    (void)length;
    if (state->error)
    {
        return false;
    }
    Read_state_set_error(state, "This build of libkunquat does not"
                                " support WavPack");
    return false;
}

#else // WITH_WAVPACK


#include <wavpack/wavpack.h>


typedef struct String_context
{
    char* data;
    uint32_t length;
    uint32_t pos;
    int push_back;
} String_context;


static int32_t read_bytes_str(void* id, void* data, int32_t bcount);
static uint32_t get_pos_str(void* id);
static int set_pos_abs_str(void* id, uint32_t pos);
static int set_pos_rel_str(void* id, int32_t delta, int mode);
static int push_back_byte_str(void* id, int c);
static uint32_t get_length_str(void* id);
static int can_seek_str(void* id);
static int32_t write_bytes_str(void* id, void* data, int32_t bcount);


static int32_t read_bytes_str(void* id, void* data, int32_t bcount)
{
    assert(id != NULL);
    assert(data != NULL);
    String_context* sc = id;
    if (sc->pos >= sc->length)
    {
        return 0;
    }
    int32_t left = sc->length - sc->pos;
    int32_t bytes_read = MIN(left, bcount);
    if (sc->push_back != EOF)
    {
        *((char*)data) = sc->push_back;
        memcpy((char*)data + 1, sc->data + sc->pos + 1, bytes_read - 1);
        sc->push_back = EOF;
    }
    else
    {
        memcpy(data, sc->data + sc->pos, bytes_read);
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
    return bytes_read;
}


static uint32_t get_pos_str(void* id)
{
    assert(id != NULL);
    String_context* sc = id;
    return sc->pos;
}


static int set_pos_abs_str(void* id, uint32_t pos)
{
    assert(id != NULL);
    String_context* sc = id;
    if (pos > sc->length)
    {
        sc->pos = sc->length;
    }
    else
    {
        sc->pos = pos;
    }
    sc->push_back = EOF;
    return 0;
}


static int set_pos_rel_str(void* id, int32_t delta, int mode)
{
    assert(id != NULL);
    String_context* sc = id;
    int32_t ref = 0;
    if (mode == SEEK_SET)
    {
        ref = 0;
    }
    else if (mode == SEEK_CUR)
    {
        ref = sc->pos;
    }
    else if (mode == SEEK_END)
    {
        ref = sc->length;
    }
    else
    {
        return -1;
    }
    sc->push_back = EOF;
    ref += delta;
    if (ref < 0)
    {
        ref = 0;
    }
    else if (ref > (int32_t)sc->length)
    {
        ref = sc->length;
    }
    sc->pos = ref;
    return 0;
}


static int push_back_byte_str(void* id, int c)
{
//    fprintf(stderr, "push back %c\n", (char)c);
    assert(id != NULL);
    String_context* sc = id;
    if (sc->push_back != EOF)
    {
        return EOF;
    }
    if (sc->pos == 0)
    {
        return EOF;
    }
    --sc->pos;
    sc->push_back = c;
    return c;
}


static uint32_t get_length_str(void* id)
{
    assert(id != NULL);
    String_context* sc = id;
    return sc->length;
}


static int can_seek_str(void* id)
{
    assert(id != NULL);
    (void)id;
    return 1;
}


static int32_t write_bytes_str(void* id, void* data, int32_t bcount)
{
    (void)id;
    (void)data;
    (void)bcount;
    assert(false);
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


#define read_wp_samples(count, offset, buf_l, buf_r, src, channels, lshift) \
    if (true)                                                               \
    {                                                                       \
        assert(buf_l != NULL);                                              \
        for (uint32_t i = 0; i < count; ++i)                                \
        {                                                                   \
            buf_l[offset + i] = src[i * channels] << lshift;                \
        }                                                                   \
        if (channels == 2)                                                  \
        {                                                                   \
            assert(buf_r != NULL);                                          \
            for (uint32_t i = 0; i < count; ++i)                            \
            {                                                               \
                buf_r[offset + i] = src[i * channels + 1] << lshift;        \
            }                                                               \
        }                                                                   \
    } else (void)0

bool Sample_parse_wavpack(Sample* sample,
                          void* data,
                          long length,
                          Read_state* state)
{
    assert(sample != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    String_context* sc = &(String_context) {
                             .data = data,
                             .length = length,
                             .pos = 0,
                             .push_back = EOF
                         };
    char err_str[80] = { '\0' };
    WavpackContext* context = WavpackOpenFileInputEx(&reader_str,
                                                     sc,
                                                     NULL,
                                                     err_str,
                                                     OPEN_2CH_MAX | OPEN_NORMALIZE,
                                                     0);
    if (context == NULL)
    {
        Read_state_set_error(state, err_str);
        return false;
    }
    int mode = WavpackGetMode(context);
    int channels = WavpackGetReducedChannels(context);
//    uint32_t freq = WavpackGetSampleRate(context);
    int bits = WavpackGetBitsPerSample(context);
    int bytes = WavpackGetBytesPerSample(context);
    uint32_t len = WavpackGetNumSamples(context);
//    uint32_t file_size = WavpackGetFileSize(context);
    if (len == (uint32_t)-1)
    {
        WavpackCloseFile(context);
        Read_state_set_error(state, "Couldn't determine WavPack"
                " file length");
        return false;
    }
    sample->params.format = SAMPLE_FORMAT_WAVPACK;
    sample->len = len;
    sample->channels = channels;
//    sample->params.mid_freq = freq;
//    Sample_set_loop(sample, SAMPLE_LOOP_OFF);
//    Sample_set_loop_start(sample, 0);
//    Sample_set_loop_end(sample, 0);
    if (bits <= 8)
    {
        sample->bits = 8;
    }
    else if (bits <= 16)
    {
        sample->bits = 16;
    }
    else if (bits <= 24)
    {
        sample->bits = 24;
    }
    else
    {
        sample->bits = 32;
    }
    if ((mode & MODE_FLOAT))
    {
        sample->is_float = true;
        sample->bits = 32;
    }
    int req_bytes = sample->bits / 8;
    sample->data[0] = sample->data[1] = NULL;
    void* nbuf_l = xnalloc(char, sample->len * req_bytes);
    if (nbuf_l == NULL)
    {
        WavpackCloseFile(context);
        return false;
    }
    if (channels == 2)
    {
        void* nbuf_r = xnalloc(char, sample->len * req_bytes);
        if (nbuf_r == NULL)
        {
            xfree(nbuf_l);
            WavpackCloseFile(context);
            return false;
        }
        sample->data[1] = nbuf_r;
    }
    sample->data[0] = nbuf_l;
    int32_t buf[256] = { 0 };
    uint32_t read = WavpackUnpackSamples(context, buf,
            256 / sample->channels);
    uint32_t written = 0;
    while (read > 0 && written < sample->len)
    {
        if (req_bytes == 1)
        {
            int8_t* buf_l = sample->data[0];
            int8_t* buf_r = NULL;
            if (sample->channels == 2)
            {
                buf_r = sample->data[1];
            }
            read_wp_samples(read, written, buf_l, buf_r, buf,
                    sample->channels, 0);
        }
        else if (req_bytes == 2)
        {
            int16_t* buf_l = sample->data[0];
            int16_t* buf_r = NULL;
            if (sample->channels == 2)
            {
                buf_r = sample->data[1];
            }
            read_wp_samples(read, written, buf_l, buf_r, buf,
                    sample->channels, 0);
        }
        else
        {
            if (sample->is_float)
            {
                float* buf_l = sample->data[0];
                float* buf_r = NULL;
                if (sample->channels == 2)
                {
                    buf_r = sample->data[1];
                }
                read_wp_samples(read, written, buf_l, buf_r, buf,
                        sample->channels, 0);
            }
            else
            {
                int32_t* buf_l = sample->data[0];
                int32_t* buf_r = NULL;
                int shift = 0;
                if (bytes == 3)
                {
                    shift = 8;
                }
                if (sample->channels == 2)
                {
                    buf_r = sample->data[1];
                }
                read_wp_samples(read, written, buf_l, buf_r, buf,
                        sample->channels, shift);
            }
        }
        written += read;
        read = WavpackUnpackSamples(context, buf, 256 / sample->channels);
    }
    WavpackCloseFile(context);
    if (written < sample->len)
    {
        Read_state_set_error(state, "Couldn't read all sample data");
        xfree(sample->data[0]);
        xfree(sample->data[1]);
        sample->data[0] = sample->data[1] = NULL;
        sample->len = 0;
        return false;
    }
    return true;
}

#undef read_wp_samples


#endif // WITH_WAVPACK


