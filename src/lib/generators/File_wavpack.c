

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <archive.h>
#include <archive_entry.h>

#include <Sample.h>
#include <File_wavpack.h>
#include <Directory.h>
#include <Handle_private.h>
#include <math_common.h>

#include <wavpack/wavpack.h>

#include <xmemory.h>


typedef struct File_context
{
    FILE* file;
    uint64_t offset;
    kqt_Handle* handle;
} File_context;


typedef struct Tar_context
{
    struct archive* reader;
    struct archive_entry* entry;
    uint64_t pos;
    int push_back;
    kqt_Handle* handle;
} Tar_context;


typedef struct String_context
{
    char* data;
    uint32_t length;
    uint32_t pos;
    int push_back;
} String_context;


static int32_t read_bytes(void* id, void* data, int32_t bcount);
static uint32_t get_pos(void* id);
static int set_pos_abs(void* id, uint32_t pos);
static int set_pos_rel(void* id, int32_t delta, int mode);
static int push_back_byte(void* id, int c);
static uint32_t get_length(void* id);
static int can_seek(void* id);
static int32_t write_bytes(void* id, void* data, int32_t bcount);


static int32_t read_bytes_tar(void* id, void* data, int32_t bcount);
static uint32_t get_pos_tar(void* id);
static int set_pos_abs_tar(void* id, uint32_t pos);
static int set_pos_rel_tar(void* id, int32_t delta, int mode);
static int push_back_byte_tar(void* id, int c);
static uint32_t get_length_tar(void* id);
static int can_seek_tar(void* id);
static int32_t write_bytes_tar(void* id, void* data, int32_t bcount);


static int32_t read_bytes_str(void* id, void* data, int32_t bcount);
static uint32_t get_pos_str(void* id);
static int set_pos_abs_str(void* id, uint32_t pos);
static int set_pos_rel_str(void* id, int32_t delta, int mode);
static int push_back_byte_str(void* id, int c);
static uint32_t get_length_str(void* id);
static int can_seek_str(void* id);
static int32_t write_bytes_str(void* id, void* data, int32_t bcount);


static int32_t read_bytes(void* id, void* data, int32_t bcount)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    return (int32_t)fread(data, 1, bcount, fc->file);
}

static int32_t read_bytes_tar(void* id, void* data, int32_t bcount)
{
    assert(id != NULL);
    assert(data != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    char* datac = data;
    if (bcount <= 0)
    {
        return 0;
    }
    int32_t bytes_from_reader = bcount;
    int32_t ret = 0;
    if (tc->push_back != EOF)
    {
        *datac = tc->push_back;
        tc->push_back = EOF;
        ++ret;
        ++datac;
        --bytes_from_reader;
        if (bytes_from_reader == 0)
        {
            return 1;
        }
    }
    ret += archive_read_data(tc->reader, datac, bytes_from_reader);
    tc->pos += ret;
    return ret;
}

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
    char* data_ptr = sc->data;
    if (sc->push_back != EOF)
    {
        *data_ptr = sc->push_back;
        ++data_ptr;
        --bcount;
    }
    memcpy(data, data_ptr + sc->pos, bytes_read);
    sc->pos += bytes_read;
    return bytes_read;
}


static uint32_t get_pos(void* id)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    assert(ftell(fc->file) >= (int64_t)fc->offset);
    return ftell(fc->file) - fc->offset;
}

static uint32_t get_pos_tar(void* id)
{
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    return tc->pos;
}

static uint32_t get_pos_str(void* id)
{
    assert(id != NULL);
    String_context* sc = id;
    return sc->pos;
}


static int set_pos_abs(void* id, uint32_t pos)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    return fseek(fc->file, pos + fc->offset, SEEK_SET);
}

static int set_pos_abs_tar(void* id, uint32_t pos)
{
    (void)pos;
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    (void)tc;
    return -1;
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
    return 0;
}


static int set_pos_rel(void* id, int32_t delta, int mode)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    int ret = fseek(fc->file, delta, mode);
    if (ret != 0)
    {
        return ret;
    }
    if (ftell(fc->file) < (int64_t)fc->offset)
    {
        return fseek(fc->file, fc->offset, SEEK_SET);
    }
    return ret;
}

static int set_pos_rel_tar(void* id, int32_t delta, int mode)
{
    (void)delta;
    (void)mode;
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    (void)tc;
    return -1;
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


static int push_back_byte(void* id, int c)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    return ungetc(c, fc->file);
}

static int push_back_byte_tar(void* id, int c)
{
    (void)c;
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    if (tc->push_back != EOF)
    {
        return EOF;
    }
    if (tc->pos == 0)
    {
        return EOF;
    }
    --tc->pos;
    tc->push_back = c;
    return c;
}

static int push_back_byte_str(void* id, int c)
{
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


static uint32_t get_length(void* id)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    return (uint32_t)file_size(fc->file, fc->handle);
}

static uint32_t get_length_tar(void* id)
{
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    return archive_entry_size(tc->entry);
}

static uint32_t get_length_str(void* id)
{
    assert(id != NULL);
    String_context* sc = id;
    return sc->length;
}


static int can_seek(void* id)
{
    assert(id != NULL);
    File_context* fc = id;
    assert(fc->file != NULL);
    return file_info(fc->file, fc->handle) == PATH_IS_REGULAR;
}

static int can_seek_tar(void* id)
{
    assert(id != NULL);
    Tar_context* tc = id;
    assert(tc->reader != NULL);
    assert(tc->entry != NULL);
    (void)tc;
    return 0;
}

static int can_seek_str(void* id)
{
    assert(id != NULL);
    (void)id;
    return 1;
}


static int32_t write_bytes(void* id, void* data, int32_t bcount)
{
    (void)id;
    (void)data;
    (void)bcount;
    assert(false);
    return 0;
}

static int32_t write_bytes_tar(void* id, void* data, int32_t bcount)
{
    (void)id;
    (void)data;
    (void)bcount;
    assert(false);
    return 0;
}

static int32_t write_bytes_str(void* id, void* data, int32_t bcount)
{
    (void)id;
    (void)data;
    (void)bcount;
    assert(false);
    return 0;
}


static WavpackStreamReader reader_file =
{
    read_bytes,
    get_pos,
    set_pos_abs,
    set_pos_rel,
    push_back_byte,
    get_length,
    can_seek,
    write_bytes
};

static WavpackStreamReader reader_tar =
{
    read_bytes_tar,
    get_pos_tar,
    set_pos_abs_tar,
    set_pos_rel_tar,
    push_back_byte_tar,
    get_length_tar,
    can_seek_tar,
    write_bytes_tar
};

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
        for (uint32_t i = 0; i < count / channels; ++i)                     \
        {                                                                   \
            buf_l[offset + i] = src[i * channels] << lshift;                \
        }                                                                   \
        if (channels == 2)                                                  \
        {                                                                   \
            assert(buf_r != NULL);                                          \
            for (uint32_t i = 0; i < count / channels; ++i)                 \
            {                                                               \
                buf_r[offset + i] = src[i * channels + 1];                  \
            }                                                               \
        }                                                                   \
    } else (void)0


bool Sample_parse_wavpack(Sample* sample,
                          void* data,
                          long length,
                          Read_state* state)
{
    assert(sample != NULL);
    assert(data != NULL || length == 0);
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


bool File_wavpack_load_sample(Sample* sample, FILE* in,
                              struct archive* reader,
                              struct archive_entry* entry,
                              kqt_Handle* handle)
{
    assert(sample != NULL);
    assert((in != NULL) != (reader != NULL && entry != NULL));
    WavpackContext* context = NULL;
    char err_str[80] = { '\0' };
    File_context fc = { .file = in, .handle = handle };
    Tar_context tc = { .reader = reader, .handle = handle };
    if (in != NULL)
    {
        fc.offset = ftell(in);
        context = WavpackOpenFileInputEx(&reader_file,
                &fc,
                NULL,
                err_str,
                OPEN_2CH_MAX | OPEN_NORMALIZE,
                0);
    }
    else
    {
        assert(reader != NULL);
        assert(entry != NULL);
        tc.reader = reader;
        tc.entry = entry;
        tc.pos = 0;
        tc.push_back = EOF;
        context = WavpackOpenFileInputEx(&reader_tar,
                &tc,
                NULL,
                err_str,
                OPEN_2CH_MAX | OPEN_NORMALIZE,
                0);
    }
    if (context == NULL)
    {
        kqt_Handle_set_error(handle, "%s: %s\n", __func__, err_str);
        return false;
    }
    int mode = WavpackGetMode(context);
    int channels = WavpackGetReducedChannels(context);
    uint32_t freq = WavpackGetSampleRate(context);
    int bits = WavpackGetBitsPerSample(context);
    int bytes = WavpackGetBytesPerSample(context);
    uint32_t len = WavpackGetNumSamples(context);
//  uint32_t file_size = WavpackGetFileSize(context);
    if (len == (uint32_t)-1)
    {
        WavpackCloseFile(context);
        kqt_Handle_set_error(handle, "%s: Couldn't determine WavPack"
                " file length", __func__);
        return false;
    }
    sample->params.format = SAMPLE_FORMAT_WAVPACK;
    sample->len = len;
    sample->channels = channels;
    sample->params.mid_freq = freq;
    Sample_set_loop(sample, SAMPLE_LOOP_OFF);
    Sample_set_loop_start(sample, 0);
    Sample_set_loop_end(sample, 0);
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
    void* nbuf_l = xnalloc(char, sample->len * req_bytes);
    if (nbuf_l == NULL)
    {
        kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                __func__);
        WavpackCloseFile(context);
        return false;
    }
    if (channels == 2)
    {
        void* nbuf_r = xnalloc(char, sample->len * req_bytes);
        if (nbuf_r == NULL)
        {
            kqt_Handle_set_error(handle, "%s: Couldn't allocate memory",
                    __func__);
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
        kqt_Handle_set_error(handle, "%s: Couldn't read all sample data",
                __func__);
        xfree(sample->data[0]);
        xfree(sample->data[1]);
        sample->data[0] = sample->data[1] = NULL;
        return false;
    }
    return true;
}


