

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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
#include <Device_impl.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_delay.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


#define MAX_BUF_TIME 60

#define DB_MAX 18
#define TAPS_MAX 32


typedef struct Tap_state
{
    bool enabled;
    int32_t buf_pos;
    int32_t frames_left;
    double scale;
} Tap_state;


typedef struct Delay_state
{
    DSP_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
    Tap_state tap_states[TAPS_MAX];
} Delay_state;


typedef struct Tap
{
    double delay;
    double scale;
} Tap;


typedef struct DSP_delay
{
    Device_impl parent;

    double max_delay;
    Tap taps[TAPS_MAX];
} DSP_delay;


static void Tap_state_set(
        Tap_state* tstate,
        double delay,
        double scale,
        int32_t buf_size,
        int32_t audio_rate)
{
    assert(tstate != NULL);
    assert(isfinite(scale));
    assert(buf_size > 0);
    assert(audio_rate > 0);

    if (!isfinite(delay))
    {
        tstate->enabled = false;
        return;
    }

    tstate->enabled = true;
    tstate->frames_left = 0;

    const int32_t delay_frames = delay * audio_rate;
    assert(delay_frames >= 0);
    assert(delay_frames <= buf_size);
    tstate->buf_pos = (buf_size - delay_frames) % buf_size;
    assert(tstate->buf_pos >= 0);

    tstate->scale = scale;

    return;
}


static void Delay_state_reset(Delay_state* dlstate, const Tap taps[])
{
    assert(dlstate != NULL);

    DSP_state_reset(&dlstate->parent);

    Audio_buffer_clear(
            dlstate->buf,
            0, Audio_buffer_get_size(dlstate->buf));
    dlstate->buf_pos = 0;

    const int32_t audio_rate = dlstate->parent.parent.audio_rate;
    const int32_t buf_size = Audio_buffer_get_size(dlstate->buf);

    for (int i = 0; i < TAPS_MAX; ++i)
    {
        const Tap* tap = &taps[i];
        Tap_state* tstate = &dlstate->tap_states[i];

        Tap_state_set(tstate, tap->delay, tap->scale, buf_size, audio_rate);
    }

    return;
}


static void del_Delay_state(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Delay_state* dlstate = (Delay_state*)dev_state;
    if (dlstate->buf != NULL)
    {
        del_Audio_buffer(dlstate->buf);
        dlstate->buf = NULL;
    }

    return;
}


static Device_state* DSP_delay_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void DSP_delay_reset(const Device_impl* dimpl, Device_state* dstate);

static bool DSP_delay_set_max_delay(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_delay_set_tap_delay(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_delay_set_tap_volume(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_delay_update_state_max_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_delay_update_state_tap_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static bool DSP_delay_update_state_tap_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);

static void DSP_delay_clear_history(DSP* dsp, DSP_state* dsp_state);
//static bool DSP_delay_sync(Device* device, Device_states* dstates);
//static bool DSP_delay_update_key(Device* device, const char* key);
static bool DSP_delay_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate);

//static void DSP_delay_check_params(DSP_delay* delay, Delay_state* dlstate);

static void DSP_delay_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_DSP_delay(Device_impl* dsp_impl);


Device_impl* new_DSP_delay(DSP* dsp)
{
    DSP_delay* delay = memory_alloc_item(DSP_delay);
    if (delay == NULL)
        return NULL;

    if (!Device_impl_init(&delay->parent, del_DSP_delay))
    {
        memory_free(delay);
        return NULL;
    }

    delay->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_delay_process);
#if 0
    if (!DSP_init(&delay->parent, del_DSP_delay,
                  DSP_delay_process, buffer_size, mix_rate))
    {
        memory_free(delay);
        return NULL;
    }
#endif

    Device_set_state_creator(delay->parent.device, DSP_delay_create_state);

    Device_impl_register_reset_device_state(&delay->parent, DSP_delay_reset);

    // Register key set/update handlers
    bool reg_success = true;

    reg_success &= Device_impl_register_set_float(
            &delay->parent, "p_max_delay.jsonf", 2.0, DSP_delay_set_max_delay);
    reg_success &= Device_impl_register_set_float(
            &delay->parent,
            "tap_XX/p_delay.jsonf",
            -1.0,
            DSP_delay_set_tap_delay);
    reg_success &= Device_impl_register_set_float(
            &delay->parent,
            "tap_XX/p_volume.jsonf",
            0.0,
            DSP_delay_set_tap_volume);

    reg_success &= Device_impl_register_update_state_float(
            &delay->parent,
            "p_max_delay.jsonf",
            DSP_delay_update_state_max_delay);
    reg_success &= Device_impl_register_update_state_float(
            &delay->parent,
            "tap_XX/p_delay.jsonf",
            DSP_delay_update_state_tap_delay);
    reg_success &= Device_impl_register_update_state_float(
            &delay->parent,
            "tap_XX/p_volume.jsonf",
            DSP_delay_update_state_tap_volume);

    if (!reg_success)
    {
        del_DSP_delay(&delay->parent);
        return NULL;
    }

    DSP_set_clear_history((DSP*)delay->parent.device, DSP_delay_clear_history);
    //Device_set_sync(delay->parent.device, DSP_delay_sync);
    //Device_set_update_key(delay->parent.device, DSP_delay_update_key);
    Device_impl_register_set_audio_rate(
            &delay->parent,
            DSP_delay_set_audio_rate);

    delay->max_delay = 2;

    for (int i = 0; i < TAPS_MAX; ++i)
    {
        delay->taps[i].delay = INFINITY;
        delay->taps[i].scale = 1;
    }

    Device_register_port(delay->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(delay->parent.device, DEVICE_PORT_TYPE_SEND, 0);

#if 0
    if (!DSP_delay_set_mix_rate(&delay->parent.parent, mix_rate))
    {
        del_DSP(&delay->parent);
        return NULL;
    }
#endif

    return &delay->parent;
}


static Device_state* DSP_delay_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Delay_state* dlstate = memory_alloc_item(Delay_state);
    if (dlstate == NULL)
        return NULL;

    const DSP_delay* delay = (DSP_delay*)device->dimpl;

    DSP_state_init(&dlstate->parent, device, audio_rate, audio_buffer_size);
    dlstate->parent.parent.destroy = del_Delay_state;
    dlstate->buf = NULL;

    dlstate->buf = new_Audio_buffer(delay->max_delay * audio_rate + 1);
    if (dlstate->buf == NULL)
    {
        del_Delay_state(&dlstate->parent.parent);
        return NULL;
    }

    dlstate->buf_pos = 0;

    for (int i = 0; i < TAPS_MAX; ++i)
    {
        Tap_state* tstate = &dlstate->tap_states[i];
        tstate->enabled = false;
        tstate->buf_pos = 0;
        tstate->frames_left = 0;
        tstate->scale = 1.0;
    }

    return &dlstate->parent.parent;
}


static double get_tap_delay(double value)
{
    return (value >= 0) ? value : INFINITY;
}

static double get_tap_volume(double value)
{
    return (isfinite(value) && value < DB_MAX) ? exp2(value / 6) : 1.0;
}


static bool DSP_delay_set_max_delay(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);
    (void)indices;

    DSP_delay* delay = (DSP_delay*)dimpl;
    delay->max_delay = MIN(value, MAX_BUF_TIME);

    return true;
}


static bool DSP_delay_set_tap_delay(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= TAPS_MAX)
        return true;

    DSP_delay* delay = (DSP_delay*)dimpl;
    delay->taps[indices[0]].delay = get_tap_delay(value);

    return true;
}


static bool DSP_delay_set_tap_volume(
        Device_impl* dimpl,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= TAPS_MAX)
        return true;

    DSP_delay* delay = (DSP_delay*)dimpl;
    delay->taps[indices[0]].scale = get_tap_volume(value);

    return true;
}


static bool DSP_delay_update_state_max_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    (void)indices;

    Delay_state* dlstate = (Delay_state*)dstate;

    long buf_len = MAX(
            Audio_buffer_get_size(dlstate->buf),
            value * dstate->audio_rate + 1); // + 1 for maximum delay support
    assert(buf_len > 0);

    return Audio_buffer_resize(dlstate->buf, buf_len);
}


static bool DSP_delay_update_state_tap_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= TAPS_MAX)
        return true;

    DSP_delay* delay = (DSP_delay*)dimpl;
    Delay_state* dlstate = (Delay_state*)dstate;
    Tap_state_set(
            &dlstate->tap_states[indices[0]],
            value,
            delay->taps[indices[0]].scale,
            Audio_buffer_get_size(dlstate->buf),
            dstate->audio_rate);

    return true;
}


static bool DSP_delay_update_state_tap_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);
    (void)dimpl;

    if (indices[0] < 0 || indices[0] >= TAPS_MAX)
        return true;

    Delay_state* dlstate = (Delay_state*)dstate;
    dlstate->tap_states[indices[0]].scale = get_tap_volume(value);

    return true;
}


static void DSP_delay_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    DSP_delay* delay = (DSP_delay*)dimpl;
    DSP_state* dsp_state = (DSP_state*)dstate;

    DSP_delay_clear_history((DSP*)delay->parent.device, dsp_state);

    return;
}


static void DSP_delay_clear_history(DSP* dsp, DSP_state* dsp_state)
{
    assert(dsp != NULL);
    //assert(string_eq(dsp->type, "delay"));
    assert(dsp_state != NULL);

    Delay_state* dlstate = (Delay_state*)dsp_state;
    Audio_buffer_clear(dlstate->buf, 0, Audio_buffer_get_size(dlstate->buf));

    return;
}


#if 0
static bool DSP_delay_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    if (!DSP_delay_update_key(device, "p_max_delay.jsonf"))
        return false;

    return true;
}


static bool DSP_delay_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);

    DSP_delay* delay = (DSP_delay*)device->dimpl;
    Device_params* params = ((DSP*)delay->parent.device)->conf->params;

    if (string_eq(key, "p_max_delay.jsonf"))
    {
        double* delay_param = Device_params_get_float(params, key);
        if (delay_param != NULL)
            delay->max_delay = *delay_param;
        else
            delay->max_delay = 2;
    }

    return true;
}
#endif


static bool DSP_delay_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    const DSP_delay* delay = (DSP_delay*)dimpl;
    Delay_state* dlstate = (Delay_state*)dstate;

    assert(dlstate->buf != NULL);
    long buf_len = MAX(
            Audio_buffer_get_size(dlstate->buf),
            delay->max_delay * audio_rate);
    assert(buf_len > 0);
    buf_len += 1; // so that the maximum delay will work

    if (!Audio_buffer_resize(dlstate->buf, buf_len))
        return false;

    Delay_state_reset(dlstate, delay->taps);

    return true;
}


static void DSP_delay_process(
        Device* device,
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

    Delay_state* dlstate = (Delay_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));

    DSP_delay* delay = (DSP_delay*)device->dimpl;
    //assert(string_eq(delay->parent.type, "delay"));
    //DSP_delay_check_params(delay, dlstate);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(&dlstate->parent.parent, 0, in_data);
    DSP_get_raw_output(&dlstate->parent.parent, 0, out_data);
    kqt_frame* delay_data[] =
    {
        Audio_buffer_get_buffer(dlstate->buf, 0),
        Audio_buffer_get_buffer(dlstate->buf, 1)
    };

    int32_t buf_size = Audio_buffer_get_size(dlstate->buf);
    assert(start <= until);
    int32_t nframes = until - start;

    for (int tap_index = 0; tap_index < TAPS_MAX; ++tap_index)
    {
        const Tap* tap = &delay->taps[tap_index];
        if (tap->delay > delay->max_delay)
            continue;

        Tap_state* tstate = &dlstate->tap_states[tap_index];

        int32_t delay_frames = dlstate->buf_pos - tstate->buf_pos;
        if (delay_frames < 0)
        {
            delay_frames += buf_size;
            assert(delay_frames > 0);
        }

        tstate->frames_left = nframes;
        int32_t mix_until = MIN(delay_frames, nframes);

        for (int32_t i = 0; i < mix_until; ++i)
        {
            out_data[0][start + i] +=
                delay_data[0][tstate->buf_pos] * tap->scale;
            out_data[1][start + i] +=
                delay_data[1][tstate->buf_pos] * tap->scale;

            ++tstate->buf_pos;
            if (tstate->buf_pos >= buf_size)
            {
                assert(tstate->buf_pos == buf_size);
                tstate->buf_pos = 0;
            }
        }

        tstate->frames_left -= mix_until;
    }

    int32_t new_buf_pos = dlstate->buf_pos;

    for (uint32_t i = start; i < until; ++i)
    {
        delay_data[0][new_buf_pos] = in_data[0][i];
        delay_data[1][new_buf_pos] = in_data[1][i];

        ++new_buf_pos;
        if (new_buf_pos >= buf_size)
        {
            assert(new_buf_pos == buf_size);
            new_buf_pos = 0;
        }
    }

    for (int tap_index = 0; tap_index < TAPS_MAX; ++tap_index)
    {
        Tap* tap = &delay->taps[tap_index];
        if (tap->delay > delay->max_delay)
            continue;

        Tap_state* tstate = &dlstate->tap_states[tap_index];

        for (int32_t i = nframes - tstate->frames_left; i < nframes; ++i)
        {
            out_data[0][start + i] +=
                delay_data[0][tstate->buf_pos] * tap->scale;
            out_data[1][start + i] +=
                delay_data[1][tstate->buf_pos] * tap->scale;

            ++tstate->buf_pos;
            if (tstate->buf_pos >= buf_size)
            {
                assert(tstate->buf_pos == buf_size);
                tstate->buf_pos = 0;
            }
        }
    }

    dlstate->buf_pos += nframes;
    if (dlstate->buf_pos >= buf_size)
    {
        dlstate->buf_pos -= buf_size;
        assert(dlstate->buf_pos >= 0);
        assert(dlstate->buf_pos < buf_size);
    }

    return;
}


#if 0
static void DSP_delay_check_params(DSP_delay* delay, Delay_state* dlstate)
{
    assert(delay != NULL);
    assert(((DSP*)delay->parent.device)->conf != NULL);
    assert(dlstate != NULL);

    Device_params* params = ((DSP*)delay->parent.device)->conf->params;
    assert(params != NULL);

    char delay_key[] = "tap_XX/p_delay.jsonf";
    int delay_key_bytes = strlen(delay_key) + 1;
    char vol_key[] = "tap_XX/p_volume.jsonf";
    int vol_key_bytes = strlen(vol_key) + 1;
    uint32_t mix_rate = dlstate->parent.parent.audio_rate;
    uint32_t buf_size = Audio_buffer_get_size(dlstate->buf);

    for (int i = 0; i < TAPS_MAX; ++i)
    {
        Tap* tap = &delay->taps[i];
        Tap_state* tstate = &dlstate->tap_states[i];

        snprintf(delay_key, delay_key_bytes, "tap_%02x/p_delay.jsonf", i);
        double* delay_param = Device_params_get_float(params, delay_key);
        if (delay_param != NULL && *delay_param >= 0)
        {
            if (tap->delay != *delay_param)
            {
                tap->delay = *delay_param;
                if (tap->delay <= delay->max_delay)
                {
                    int32_t delay_frames = tap->delay * mix_rate;
                    assert(delay_frames >= 0);
                    assert((uint32_t)delay_frames <= buf_size);
                    tstate->buf_pos =
                        (buf_size + dlstate->buf_pos - delay_frames) % buf_size;
                    assert(tstate->buf_pos >= 0);
                }
            }
        }
        else
        {
            tap->delay = INFINITY;
        }

        snprintf(vol_key, vol_key_bytes, "tap_%02x/p_volume.jsonf", i);
        double* vol = Device_params_get_float(params, vol_key);
        if (vol != NULL && isfinite(*vol) && *vol < DB_MAX)
            tap->scale = exp2(*vol / 6);
        else
            tap->scale = 1;
    }

    return;
}
#endif


static void del_DSP_delay(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "delay"));
    DSP_delay* delay = (DSP_delay*)dsp_impl;
    memory_free(delay);

    return;
}


