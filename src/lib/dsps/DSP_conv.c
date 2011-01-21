

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <Audio_buffer.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_conv.h>
#include <math_common.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


#define DEFAULT_IR_LEN 0.25


typedef struct DSP_conv
{
    DSP parent;
    Audio_buffer* ir;
    Audio_buffer* history;
    double max_ir_len;
    int32_t actual_ir_len;
    int32_t history_pos;
    int32_t ir_rate;
} DSP_conv;


static void DSP_conv_set_ir(DSP_conv* conv);

static void DSP_conv_reset(Device* device);
static bool DSP_conv_sync(Device* device);
static void DSP_conv_clear_history(DSP* dsp);
static bool DSP_conv_update_key(Device* device, const char* key);
static bool DSP_conv_set_mix_rate(Device* device, uint32_t mix_rate);

static void DSP_conv_process(Device* device,
                             uint32_t start,
                             uint32_t until,
                             uint32_t freq,
                             double tempo);

static void DSP_conv_check_params(DSP_conv* conv);

static void del_DSP_conv(DSP* dsp);


DSP* new_DSP_conv(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    DSP_conv* conv = xalloc(DSP_conv);
    if (conv == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&conv->parent, del_DSP_conv,
                  DSP_conv_process, buffer_size, mix_rate))
    {
        xfree(conv);
        return NULL;
    }
    DSP_set_clear_history(&conv->parent, DSP_conv_clear_history);
    Device_set_reset(&conv->parent.parent, DSP_conv_reset);
    Device_set_sync(&conv->parent.parent, DSP_conv_sync);
    Device_set_update_key(&conv->parent.parent, DSP_conv_update_key);
    conv->ir = NULL;
    conv->history = NULL;
    conv->max_ir_len = DEFAULT_IR_LEN;
    conv->actual_ir_len = 0;
    conv->history_pos = 0;
    conv->ir_rate = 48000;
    Device_register_port(&conv->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&conv->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    if (!DSP_conv_set_mix_rate(&conv->parent.parent, mix_rate))
    {
        del_DSP(&conv->parent);
        return NULL;
    }
    return &conv->parent;
}


static void DSP_conv_reset(Device* device)
{
    assert(device != NULL);
    DSP_reset(device);
    DSP_conv* conv = (DSP_conv*)device;
    DSP_conv_clear_history(&conv->parent);
    return;
}


static bool DSP_conv_sync(Device* device)
{
    assert(device != NULL);
    if (!DSP_conv_update_key(device, "p_max_ir_len.jsonf") ||
            !DSP_conv_update_key(device, "p_ir.wv"))
    {
        return false;
    }
    return true;
}


static void DSP_conv_clear_history(DSP* dsp)
{
    assert(dsp != NULL);
    assert(string_eq(dsp->type, "convolution"));
    DSP_conv* conv = (DSP_conv*)dsp;
    Audio_buffer_clear(conv->history, 0,
                       Audio_buffer_get_size(conv->history));
    return;
}


static bool DSP_conv_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);
    DSP_conv* conv = (DSP_conv*)device;
    Device_params* params = conv->parent.conf->params;
    if (string_eq(key, "p_max_ir_len.jsonf"))
    {
        double* max_param = Device_params_get_float(params, key);
        if (max_param != NULL)
        {
            conv->max_ir_len = *max_param;
        }
        else
        {
            conv->max_ir_len = DEFAULT_IR_LEN;
        }
        if (!DSP_conv_set_mix_rate(device, Device_get_mix_rate(device)))
        {
            return false;
        }
    }
    else if (string_eq(key, "p_ir.wv") || string_eq(key, "p_volume.jsonf"))
    {
        if (!DSP_conv_set_mix_rate(device, Device_get_mix_rate(device)))
        {
            return false;
        }
    }
    return true;
}


static bool DSP_conv_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    DSP_conv* conv = (DSP_conv*)device;
    long buf_size = conv->max_ir_len * mix_rate;
    if (buf_size <= 0)
    {
        buf_size = 1;
    }
    if (conv->ir == NULL)
    {
        conv->ir = new_Audio_buffer(buf_size);
        if (conv->ir == NULL)
        {
            return false;
        }
    }
    else if (!Audio_buffer_resize(conv->ir, buf_size))
    {
        return false;
    }
    if (conv->history == NULL)
    {
        conv->history = new_Audio_buffer(buf_size);
        if (conv->history == NULL)
        {
            return false;
        }
    }
    else if (!Audio_buffer_resize(conv->history, buf_size))
    {
        return false;
    }
    Audio_buffer_clear(conv->ir, 0, buf_size);
    Audio_buffer_clear(conv->history, 0, buf_size);
    conv->history_pos = 0;
    DSP_conv_set_ir(conv);
    assert(conv->actual_ir_len <= buf_size);
    return true;
}


#define get_values(type, divisor)                                 \
    if (true)                                                     \
    {                                                             \
        val_r = val_l = ((type*)sample->data[0])[sample_pos];     \
        if (sample->channels > 1)                                 \
        {                                                         \
            val_r = ((type*)sample->data[1])[sample_pos];         \
        }                                                         \
        if (next_pos < (int32_t)sample->len)                      \
        {                                                         \
            next_r = next_l = ((type*)sample->data[0])[next_pos]; \
            if (sample->channels > 1)                             \
            {                                                     \
                next_r = ((type*)sample->data[1])[next_pos];      \
            }                                                     \
        }                                                         \
        val_l /= divisor;                                         \
        val_r /= divisor;                                         \
        next_l /= divisor;                                        \
        next_r /= divisor;                                        \
    } else (void)0

static void DSP_conv_set_ir(DSP_conv* conv)
{
    assert(conv != NULL);
    if (conv->parent.conf == NULL)
    {
        conv->actual_ir_len = 0;
        return;
    }
    Device_params* params = conv->parent.conf->params;
    assert(params != NULL);
    Sample* sample = Device_params_get_sample(params, "p_ir.wv");
    if (sample == NULL)
    {
        conv->actual_ir_len = 0;
        return;
    }
    double scale = 1;
    double* dB_param = Device_params_get_float(params, "p_volume.jsonf");
    if (dB_param != NULL)
    {
        scale = exp2(*dB_param / 6);
    }
    conv->ir_rate = 48000; // FIXME
    double mix_rate = Device_get_mix_rate((Device*)conv);
    int32_t ir_size = Audio_buffer_get_size(conv->ir);
    kqt_frame* ir_data[] = { Audio_buffer_get_buffer(conv->ir, 0),
                             Audio_buffer_get_buffer(conv->ir, 1) };
    int32_t i = 0;
    for (; i < ir_size; ++i)
    {
        double ideal_pos = (double)i * (conv->ir_rate / mix_rate);
        int32_t sample_pos = (int32_t)ideal_pos;
        int32_t next_pos = sample_pos + 1;
        double remainder = ideal_pos - sample_pos;
        assert(remainder >= 0);
        assert(remainder < 1);
        if (sample_pos >= (int32_t)sample->len)
        {
            break;
        }
        double val_l = 0;
        double val_r = 0;
        double next_l = 0;
        double next_r = 0;
        if (sample->is_float)
        {
            get_values(float, 1);
        }
        else if (sample->bits == 8)
        {
            get_values(int8_t, 0x80);
        }
        else if (sample->bits == 16)
        {
            get_values(int16_t, 0x8000L);
        }
        else
        {
            assert(sample->bits == 32);
            get_values(int32_t, 0x80000000LL);
        }
        // TODO: improve the scaling quality
        ir_data[0][i] = (val_l + (next_l - val_l) * remainder) * scale;
        ir_data[1][i] = (val_r + (next_r - val_r) * remainder) * scale;
    }
    conv->actual_ir_len = i;
    return;
}

#undef get_values


static void DSP_conv_process(Device* device,
                             uint32_t start,
                             uint32_t until,
                             uint32_t freq,
                             double tempo)
{
    assert(device != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)tempo;
    DSP_conv* conv = (DSP_conv*)device;
    assert(string_eq(conv->parent.type, "convolution"));
    DSP_conv_check_params(conv);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    kqt_frame* ir_data[] = { Audio_buffer_get_buffer(conv->ir, 0),
                             Audio_buffer_get_buffer(conv->ir, 1) };
    kqt_frame* history_data[] = { Audio_buffer_get_buffer(conv->history, 0),
                                  Audio_buffer_get_buffer(conv->history, 1) };
    for (uint32_t out_pos = start; out_pos < until; ++out_pos)
    {
        kqt_frame out_l = 0;
        kqt_frame out_r = 0;
        int32_t history_pos = conv->history_pos;
        history_data[0][history_pos] = in_data[0][out_pos];
        history_data[1][history_pos] = in_data[1][out_pos];
        for (int32_t i = 0; i < conv->actual_ir_len; ++i)
        {
            out_l += history_data[0][history_pos] * ir_data[0][i];
            out_r += history_data[1][history_pos] * ir_data[1][i];
            --history_pos;
            if (history_pos < 0)
            {
                history_pos = conv->actual_ir_len - 1;
            }
        }
        out_data[0][out_pos] += out_l;
        out_data[1][out_pos] += out_r;
        ++conv->history_pos;
        if (conv->history_pos >= conv->actual_ir_len)
        {
            conv->history_pos = 0;
        }
    }
    return;
}


static void DSP_conv_check_params(DSP_conv* conv)
{
    assert(conv != NULL);
    assert(conv->parent.conf != NULL);
    return;
}


static void del_DSP_conv(DSP* dsp)
{
    if (dsp == NULL)
    {
        return;
    }
    assert(string_eq(dsp->type, "convolution"));
    DSP_conv* conv = (DSP_conv*)dsp;
    del_Audio_buffer(conv->ir);
    del_Audio_buffer(conv->history);
    xfree(conv);
    return;
}


