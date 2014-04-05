

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
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
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/DSP.h>
#include <devices/dsps/DSP_common.h>
#include <devices/dsps/DSP_conv.h>
#include <math_common.h>
#include <memory.h>
#include <string/common.h>


#define MAX_BUF_TIME 5

#define DEFAULT_IR_LEN 0.25


typedef struct DSP_conv
{
    Device_impl parent;

    double max_ir_len;
    int32_t ir_rate;
    double scale;

    Audio_buffer* ir;
    int32_t actual_ir_len;
} DSP_conv;


typedef struct Conv_state
{
    DSP_state parent;

    Audio_buffer* history;
    int32_t history_pos;

    double scale;
} Conv_state;


static void Conv_state_reset(Conv_state* cstate, const DSP_conv* conv)
{
    assert(cstate != NULL);
    assert(conv != NULL);

    DSP_state_reset(&cstate->parent);

    Audio_buffer_clear(
            cstate->history,
            0, Audio_buffer_get_size(cstate->history));
    cstate->history_pos = 0;

    cstate->scale = conv->scale;

    return;
}


static void del_Conv_state(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Conv_state* cstate = (Conv_state*)dev_state;
    if (cstate->history != NULL)
    {
        del_Audio_buffer(cstate->history);
        cstate->history = NULL;
    }

    return;
}


static Device_state* DSP_conv_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void DSP_conv_update_ir(DSP_conv* conv);

static void DSP_conv_reset(const Device_impl* dimpl, Device_state* dstate);

static bool DSP_conv_set_max_ir_len(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value);

static bool DSP_conv_set_ir(
        Device_impl* dimpl,
        Device_key_indices indices,
        const Sample* value);

static bool DSP_conv_set_volume(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value);

static bool DSP_conv_set_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value);

static bool DSP_conv_set_state_max_ir_len(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value);

static void DSP_conv_update_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value);

static void DSP_conv_clear_history(
        const Device_impl* dimpl, DSP_state* dsp_state);

static bool DSP_conv_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate);

static void DSP_conv_process(
        const Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_DSP_conv(Device_impl* dsp_impl);


Device_impl* new_DSP_conv(DSP* dsp)
{
    assert(false); // FIXME: This DSP is broken, fix.

    DSP_conv* conv = memory_alloc_item(DSP_conv);
    if (conv == NULL)
        return NULL;

    if (!Device_impl_init(&conv->parent, del_DSP_conv))
    {
        memory_free(conv);
        return NULL;
    }

    conv->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_conv_process);

    Device_set_state_creator(conv->parent.device, DSP_conv_create_state);

    Device_impl_register_reset_device_state(&conv->parent, DSP_conv_reset);

    // Register key set/update handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &conv->parent,
            "p_f_max_ir_len.json",
            DEFAULT_IR_LEN,
            DSP_conv_set_max_ir_len,
            DSP_conv_set_state_max_ir_len);
    reg_success &= Device_impl_register_set_sample(
            &conv->parent, "p_ir.wv", NULL, DSP_conv_set_ir, NULL);
    reg_success &= Device_impl_register_set_float(
            &conv->parent,
            "p_f_volume.json",
            0.0,
            DSP_conv_set_volume,
            DSP_conv_set_state_volume);

    reg_success &= Device_impl_register_update_state_float(
            &conv->parent, "p_f_volume.json", DSP_conv_update_state_volume);

    if (!reg_success)
    {
        del_DSP_conv(&conv->parent);
        return NULL;
    }

    DSP_set_clear_history((DSP*)conv->parent.device, DSP_conv_clear_history);

    //Device_set_sync(conv->parent.device, DSP_conv_sync);
    Device_impl_register_set_audio_rate(
            &conv->parent, DSP_conv_set_audio_rate);

    conv->max_ir_len = DEFAULT_IR_LEN;
    conv->ir_rate = 48000;
    conv->scale = 1.0;

    conv->ir = NULL;
    conv->actual_ir_len = 0;

    Device_register_port(conv->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(conv->parent.device, DEVICE_PORT_TYPE_SEND, 0);

    return &conv->parent;
}


static Device_state* DSP_conv_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Conv_state* cstate = memory_alloc_item(Conv_state);
    if (cstate == NULL)
        return NULL;

    DSP_state_init(&cstate->parent, device, audio_rate, audio_buffer_size);
    cstate->parent.parent.destroy = del_Conv_state;

    // Sanitise fields
    cstate->history = NULL;
    cstate->history_pos = 0;
    cstate->scale = 1.0;

    const DSP_conv* conv = (const DSP_conv*)device->dimpl;

    const long buf_size = conv->max_ir_len * audio_rate;
    cstate->history = new_Audio_buffer(buf_size);
    if (cstate->history == NULL)
    {
        del_Audio_buffer(cstate->history);
        return NULL;
    }

    Conv_state_reset(cstate, conv);

    return &cstate->parent.parent;
}


static void DSP_conv_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    DSP_state* dsp_state = (DSP_state*)dstate;
    DSP_conv_clear_history(dimpl, dsp_state);

    return;
}


static bool DSP_conv_set_max_ir_len(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_conv* conv = (DSP_conv*)dimpl;
    conv->max_ir_len = (value > 0 && value <= MAX_BUF_TIME)
        ? value : DEFAULT_IR_LEN;

    DSP_conv_update_ir(conv);

    return true;
}


static bool DSP_conv_set_ir(
        Device_impl* dimpl,
        Device_key_indices indices,
        const Sample* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;
    (void)value;

    DSP_conv* conv = (DSP_conv*)dimpl;

    long buf_size = conv->max_ir_len * 48000; // FIXME: 48000
    if (buf_size <= 0)
        buf_size = 1;

    if (conv->ir == NULL)
    {
        conv->ir = new_Audio_buffer(buf_size);
        if (conv->ir == NULL)
            return false;
    }
    else if (!Audio_buffer_resize(conv->ir, buf_size))
        return false;

    DSP_conv_update_ir((DSP_conv*)dimpl);

    return true;
}


// FIXME: copypasta, define this in a common utility module
static double dB_to_scale(double vol_dB)
{
    return isfinite(vol_dB) ? exp2(vol_dB / 6) : 1.0;
}


static bool DSP_conv_set_volume(
        Device_impl* dimpl,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_conv* conv = (DSP_conv*)dimpl;
    conv->scale = dB_to_scale(value);

    return true;
}


static bool DSP_conv_set_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    DSP_conv_update_state_volume(dimpl, dstate, indices, value);

    return true;
}


static bool DSP_conv_set_state_max_ir_len(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    (void)indices;

    Conv_state* cstate = (Conv_state*)dstate;

    int32_t buf_size = MAX(
            Audio_buffer_get_size(cstate->history),
            value * Device_state_get_audio_rate(dstate));
    assert(buf_size > 0);

    if (!Audio_buffer_resize(cstate->history, buf_size))
        return false;

    return true;
}


static void DSP_conv_update_state_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Device_key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    (void)indices;

    Conv_state* cstate = (Conv_state*)dstate;
    cstate->scale = dB_to_scale(value);

    return;
}


static void DSP_conv_clear_history(
        const Device_impl* dimpl, DSP_state* dsp_state)
{
    assert(dimpl != NULL);
    //assert(string_eq(dsp->type, "convolution"));
    assert(dsp_state != NULL);
    (void)dimpl;

    Conv_state* cstate = (Conv_state*)dsp_state;
    Audio_buffer_clear(
            cstate->history,
            0, Audio_buffer_get_size(cstate->history));

    return;
}


static bool DSP_conv_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    const DSP_conv* conv = (const DSP_conv*)dimpl;
    Conv_state* cstate = (Conv_state*)dstate;

    long buf_size = conv->max_ir_len * audio_rate;
    if (buf_size <= 0)
        buf_size = 1;

    assert(cstate->history != NULL);
    if (!Audio_buffer_resize(cstate->history, buf_size))
        return false;

    Audio_buffer_clear(conv->ir, 0, buf_size);
    Audio_buffer_clear(cstate->history, 0, buf_size);
    cstate->history_pos = 0;

    return true;
}


#define get_values(type, divisor)                    \
    if (true)                                        \
    {                                                \
        val_r = val_l = ((type*)sample->data[0])[i]; \
        if (sample->channels > 1)                    \
            val_r = ((type*)sample->data[1])[i];     \
        val_l /= divisor;                            \
        val_r /= divisor;                            \
    } else (void)0

static void DSP_conv_update_ir(DSP_conv* conv)
{
    assert(conv != NULL);

    const Device_params* params = conv->parent.device->dparams;
    if (params == NULL)
    {
        conv->actual_ir_len = 0;
        return;
    }

    const Sample* sample = Device_params_get_sample(params, "p_ir.wv");
    if (sample == NULL)
    {
        conv->actual_ir_len = 0;
        return;
    }

    int32_t ir_size = Audio_buffer_get_size(conv->ir);
    kqt_frame* ir_data[] =
    {
        Audio_buffer_get_buffer(conv->ir, 0),
        Audio_buffer_get_buffer(conv->ir, 1),
    };

    conv->actual_ir_len = MIN(ir_size, (int32_t)sample->len);

    for (int32_t i = 0; i < conv->actual_ir_len; ++i)
    {
        double val_l = 0;
        double val_r = 0;

        if (sample->is_float)
            get_values(float, 1);
        else if (sample->bits == 8)
            get_values(int8_t, 0x80);
        else if (sample->bits == 16)
            get_values(int16_t, 0x8000L);
        else if (sample->bits == 32)
            get_values(int32_t, 0x80000000LL);
        else
            assert(false);

        ir_data[0][i] = val_l;
        ir_data[1][i] = val_r;
    }

    return;
}

#undef get_values


static void DSP_conv_process(
        const Device* device,
        Device_states* dstates,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Conv_state* cstate = (Conv_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));

    DSP_conv* conv = (DSP_conv*)device->dimpl;
    //assert(string_eq(conv->parent.type, "convolution"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(&cstate->parent.parent, 0, in_data);
    DSP_get_raw_output(&cstate->parent.parent, 0, out_data);

    kqt_frame* ir_data[] =
    {
        Audio_buffer_get_buffer(conv->ir, 0),
        Audio_buffer_get_buffer(conv->ir, 1),
    };
    kqt_frame* history_data[] =
    {
        Audio_buffer_get_buffer(cstate->history, 0),
        Audio_buffer_get_buffer(cstate->history, 1),
    };

    const int32_t ir_size = Audio_buffer_get_size(conv->ir);
    const int32_t history_size = Audio_buffer_get_size(cstate->history);

    const double ir_scale_fac = (double)ir_size / (double)history_size;

    for (int32_t out_pos = start; out_pos < (int32_t)until; ++out_pos)
    {
        kqt_frame out_l = 0;
        kqt_frame out_r = 0;
        int32_t history_pos = cstate->history_pos;
        history_data[0][history_pos] = in_data[0][out_pos];
        history_data[1][history_pos] = in_data[1][out_pos];

        for (int32_t i = 0; i < history_size; ++i)
        {
            // Get ir position
            double ir_pos = ir_scale_fac * i;
            int32_t ir_sample_pos = (int32_t)ir_pos;
            double ir_sample_rem = ir_pos - floor(ir_pos);

            if (ir_sample_pos >= ir_size)
                break;

            // Get ir value
            float cur_val_l = ir_data[0][ir_sample_pos];
            float cur_val_r = ir_data[1][ir_sample_pos];
            float next_val_l = 0.0;
            float next_val_r = 0.0;
            if (ir_sample_pos + 1 < ir_size)
            {
                next_val_l = ir_data[0][ir_sample_pos + 1];
                next_val_r = ir_data[1][ir_sample_pos + 1];
            }

            float ir_val_l = lerp(cur_val_l, next_val_l, ir_sample_rem);
            float ir_val_r = lerp(cur_val_r, next_val_r, ir_sample_rem);

            out_l += history_data[0][history_pos] * ir_val_l * cstate->scale;
            out_r += history_data[1][history_pos] * ir_val_r * cstate->scale;

            --history_pos;
            if (history_pos < 0)
                history_pos = conv->actual_ir_len - 1;
        }

        out_data[0][out_pos] += out_l;
        out_data[1][out_pos] += out_r;

        ++cstate->history_pos;
        if (cstate->history_pos >= conv->actual_ir_len)
            cstate->history_pos = 0;
    }

    return;
}


static void del_DSP_conv(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "convolution"));
    DSP_conv* conv = (DSP_conv*)dsp_impl;
    del_Audio_buffer(conv->ir);
    memory_free(conv);

    return;
}


