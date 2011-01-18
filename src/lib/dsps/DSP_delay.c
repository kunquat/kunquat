

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
#include <DSP_delay.h>
#include <math_common.h>
#include <string_common.h>
#include <xmemory.h>


#define DB_MAX 18
#define TAPS_MAX 32


typedef struct Tap
{
    double delay;
    double scale;
    int32_t buf_pos;
    int32_t frames_left;
} Tap;


typedef struct DSP_delay
{
    DSP parent;
    Audio_buffer* buf;
    int32_t buf_pos;
    double max_delay;
    Tap taps[TAPS_MAX];
} DSP_delay;


static void DSP_delay_reset(Device* device);
static void DSP_delay_clear_history(DSP* dsp);
static bool DSP_delay_sync(Device* device);
static bool DSP_delay_set_mix_rate(Device* device, uint32_t mix_rate);

static void DSP_delay_check_params(DSP_delay* delay);

static void DSP_delay_process(Device* device,
                              uint32_t start,
                              uint32_t until,
                              uint32_t freq,
                              double tempo);


static void del_DSP_delay(DSP* dsp);


DSP* new_DSP_delay(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);
    DSP_delay* delay = xalloc(DSP_delay);
    if (delay == NULL)
    {
        return NULL;
    }
    if (!DSP_init(&delay->parent, del_DSP_delay,
                  DSP_delay_process, buffer_size, mix_rate))
    {
        xfree(delay);
        return NULL;
    }
    DSP_set_clear_history(&delay->parent, DSP_delay_clear_history);
    Device_set_reset(&delay->parent.parent, DSP_delay_reset);
    Device_set_sync(&delay->parent.parent, DSP_delay_sync);
    delay->buf = NULL;
    delay->buf_pos = 0;
    delay->max_delay = 2;
    for (int i = 0; i < TAPS_MAX; ++i)
    {
        delay->taps[i].delay = INFINITY;
        delay->taps[i].scale = 1;
        delay->taps[i].buf_pos = 0;
    }
    Device_register_port(&delay->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&delay->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    if (!DSP_delay_set_mix_rate(&delay->parent.parent, mix_rate))
    {
        del_DSP(&delay->parent);
        return NULL;
    }
    return &delay->parent;
}


static void DSP_delay_reset(Device* device)
{
    assert(device != NULL);
    DSP_reset(device);
    DSP_delay* delay = (DSP_delay*)device;
    DSP_delay_clear_history(&delay->parent);
    return;
}


static void DSP_delay_clear_history(DSP* dsp)
{
    assert(dsp != NULL);
    assert(string_eq(dsp->type, "delay"));
    DSP_delay* delay = (DSP_delay*)dsp;
    Audio_buffer_clear(delay->buf, 0, Audio_buffer_get_size(delay->buf));
    return;
}


static bool DSP_delay_sync(Device* device)
{
    assert(device != NULL);
    DSP_delay* delay = (DSP_delay*)device;
    Device_params* params = delay->parent.conf->params;
    const char* ss_key = Device_params_get_slow_sync_key(params);
    while (ss_key != NULL)
    {
        if (string_eq(ss_key, "max_delay.jsonf"))
        {
            double* delay_param = Device_params_get_float(params, ss_key);
            assert(delay_param != NULL);
            delay->max_delay = *delay_param;
            if (!DSP_delay_set_mix_rate(device, Device_get_mix_rate(device)))
            {
                return false;
            }
        }
        ss_key = Device_params_get_slow_sync_key(params);
    }
    Device_params_synchronised(params);
    return true;
}


static bool DSP_delay_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    DSP_delay* delay = (DSP_delay*)device;
    long buf_len = MAX(Device_get_buffer_size(device),
                       delay->max_delay * mix_rate);
    assert(buf_len > 0);
    buf_len += 1; // so that the maximum delay will work
    if (delay->buf == NULL)
    {
        delay->buf = new_Audio_buffer(buf_len);
        if (delay->buf == NULL)
        {
            return false;
        }
    }
    else if (!Audio_buffer_resize(delay->buf, buf_len))
    {
        return false;
    }
    Audio_buffer_clear(delay->buf, 0, Audio_buffer_get_size(delay->buf));
    delay->buf_pos = 0;
    for (int i = 0; i < TAPS_MAX; ++i)
    {
        Tap* tap = &delay->taps[i];
        if (tap->delay > delay->max_delay)
        {
            continue;
        }
        int32_t delay_frames = tap->delay * mix_rate;
        assert(delay_frames >= 0);
        assert(delay_frames <= buf_len);
        tap->buf_pos = (buf_len - delay_frames) % buf_len;
        assert(tap->buf_pos >= 0);
    }
    return true;
}


static void DSP_delay_process(Device* device,
                              uint32_t start,
                              uint32_t until,
                              uint32_t freq,
                              double tempo)
{
    assert(device != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    (void)tempo;
    DSP_delay* delay = (DSP_delay*)device;
    assert(string_eq(delay->parent.type, "delay"));
    DSP_delay_check_params(delay);
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    kqt_frame* delay_data[] = { Audio_buffer_get_buffer(delay->buf, 0),
                                Audio_buffer_get_buffer(delay->buf, 1) };
    int32_t buf_size = Audio_buffer_get_size(delay->buf);
    assert(start <= until);
    int32_t nframes = until - start;
    for (int tap_index = 0; tap_index < TAPS_MAX; ++tap_index)
    {
        Tap* tap = &delay->taps[tap_index];
        if (tap->delay > delay->max_delay)
        {
            continue;
        }
        int32_t delay_frames = delay->buf_pos - tap->buf_pos;
        if (delay_frames < 0)
        {
            delay_frames += buf_size;
            assert(delay_frames > 0);
        }
        tap->frames_left = nframes;
        int32_t mix_until = MIN(delay_frames, nframes);
        for (int32_t i = 0; i < mix_until; ++i)
        {
            out_data[0][start + i] += delay_data[0][tap->buf_pos] *
                                      tap->scale;
            out_data[1][start + i] += delay_data[1][tap->buf_pos] *
                                      tap->scale;
            ++tap->buf_pos;
            if (tap->buf_pos >= buf_size)
            {
                assert(tap->buf_pos == buf_size);
                tap->buf_pos = 0;
            }
        }
        tap->frames_left -= mix_until;
    }
    int32_t new_buf_pos = delay->buf_pos;
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
        {
            continue;
        }
        for (int32_t i = nframes - tap->frames_left; i < nframes; ++i)
        {
            out_data[0][start + i] += delay_data[0][tap->buf_pos] *
                                      tap->scale;
            out_data[1][start + i] += delay_data[1][tap->buf_pos] *
                                      tap->scale;
            ++tap->buf_pos;
            if (tap->buf_pos >= buf_size)
            {
                assert(tap->buf_pos == buf_size);
                tap->buf_pos = 0;
            }
        }
    }
    delay->buf_pos += nframes;
    if (delay->buf_pos >= buf_size)
    {
        delay->buf_pos -= buf_size;
        assert(delay->buf_pos >= 0);
        assert(delay->buf_pos < buf_size);
    }
    return;
}


static void DSP_delay_check_params(DSP_delay* delay)
{
    assert(delay != NULL);
    assert(delay->parent.conf != NULL);
    Device_params* params = delay->parent.conf->params;
    assert(params != NULL);
    char delay_key[] = "tap_XX/p_delay.jsonf";
    int delay_key_bytes = strlen(delay_key) + 1;
    char vol_key[] = "tap_XX/p_volume.jsonf";
    int vol_key_bytes = strlen(vol_key) + 1;
    uint32_t mix_rate = Device_get_mix_rate(&delay->parent.parent);
    uint32_t buf_size = Audio_buffer_get_size(delay->buf);
    for (int i = 0; i < TAPS_MAX; ++i)
    {
        Tap* tap = &delay->taps[i];
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
                    tap->buf_pos = (buf_size - delay_frames) % buf_size;
                    assert(tap->buf_pos >= 0);
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
        {
            tap->scale = exp2(*vol / 6);
        }
        else
        {
            tap->scale = 1;
        }
    }
    return;
}


static void del_DSP_delay(DSP* dsp)
{
    if (dsp == NULL)
    {
        return;
    }
    assert(string_eq(dsp->type, "delay"));
    DSP_delay* delay = (DSP_delay*)dsp;
    del_Audio_buffer(delay->buf);
    xfree(delay);
    return;
}


