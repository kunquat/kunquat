

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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <Audio_buffer.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_chorus.h>
#include <LFO.h>
#include <math_common.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


#define CHORUS_BUF_TIME 0.25
#define CHORUS_VOICES_MAX 32
#define DB_MAX 18


typedef struct Chorus_voice
{
    double init_delay;
    double delay;
    double offset;
    LFO delay_variance;
    double init_range;
    double range;
    bool range_changed;
    double init_speed;
    double speed;
    double speed_changed;
    double init_scale;
    double scale;
    int32_t buf_pos;
} Chorus_voice;


typedef struct DSP_chorus
{
    DSP parent;
    Audio_buffer* buf;
    int32_t buf_pos;
    Chorus_voice voices[CHORUS_VOICES_MAX];
} DSP_chorus;


static void DSP_chorus_reset(Device* device);
static void DSP_chorus_clear_history(DSP* dsp);
static bool DSP_chorus_sync(Device* device);
static bool DSP_chorus_update_key(Device* device, const char* key);
static bool DSP_chorus_set_mix_rate(Device* device, uint32_t mix_rate);

static void DSP_chorus_check_params(DSP_chorus* chorus);

static void DSP_chorus_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_DSP_chorus(DSP* dsp);


DSP* new_DSP_chorus(uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    DSP_chorus* chorus = memory_alloc_item(DSP_chorus);
    if (chorus == NULL)
        return NULL;

    if (!DSP_init(&chorus->parent, del_DSP_chorus,
                  DSP_chorus_process, buffer_size, mix_rate))
    {
        memory_free(chorus);
        return NULL;
    }
    DSP_set_clear_history(&chorus->parent, DSP_chorus_clear_history);
    Device_set_reset(&chorus->parent.parent, DSP_chorus_reset);
    Device_set_sync(&chorus->parent.parent, DSP_chorus_sync);
    Device_set_update_key(&chorus->parent.parent, DSP_chorus_update_key);
    chorus->buf = NULL;
    chorus->buf_pos = 0;
    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &chorus->voices[i];
        voice->init_delay = -1;
        voice->delay = -1;
        voice->offset = 0;
        LFO_init(&voice->delay_variance, LFO_MODE_LINEAR);
        LFO_set_tempo(&voice->delay_variance, 60);
        voice->init_range = 0;
        voice->range = 0;
        voice->range_changed = false;
        voice->init_speed = 0;
        voice->speed = 0;
        voice->speed_changed = false;
        voice->init_scale = 1;
        voice->scale = 1;
        voice->buf_pos = 0;
    }
    Device_register_port(&chorus->parent.parent, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(&chorus->parent.parent, DEVICE_PORT_TYPE_SEND, 0);
    if (!DSP_chorus_set_mix_rate(&chorus->parent.parent, mix_rate))
    {
        del_DSP(&chorus->parent);
        return NULL;
    }
    return &chorus->parent;
}


static void DSP_chorus_reset(Device* device)
{
    assert(device != NULL);
    DSP_reset(device);
    DSP_chorus* chorus = (DSP_chorus*)device;
    DSP_chorus_clear_history(&chorus->parent);
    chorus->buf_pos = 0;
    uint32_t buf_size = Audio_buffer_get_size(chorus->buf);
    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &chorus->voices[i];
        voice->delay = voice->init_delay;
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
        {
            continue;
        }
        voice->offset = 0;
        voice->delay = voice->init_delay;
        voice->range_changed = voice->range != voice->init_range;
        voice->range = voice->init_range;
        voice->speed_changed = voice->speed != voice->init_speed;
        voice->speed = voice->init_speed;
        voice->scale = voice->init_scale;
        double buf_pos = voice->delay * Device_get_mix_rate(device);
        assert(buf_pos >= 0);
        assert(buf_pos < buf_size - 1);
        voice->buf_pos = fmod((buf_size - buf_pos), buf_size);
        assert(voice->buf_pos >= 0);
    }
    return;
}


static void DSP_chorus_clear_history(DSP* dsp)
{
    assert(dsp != NULL);
    assert(string_eq(dsp->type, "chorus"));
    DSP_chorus* chorus = (DSP_chorus*)dsp;
    Audio_buffer_clear(chorus->buf, 0, Audio_buffer_get_size(chorus->buf));
    return;
}


static bool DSP_chorus_sync(Device* device)
{
    assert(device != NULL);
    char delay_key[] = "voice_XX/p_delay.jsonf";
    int delay_key_bytes = strlen(delay_key) + 1;
    char range_key[] = "voice_XX/p_range.jsonf";
    int range_key_bytes = strlen(range_key) + 1;
    char speed_key[] = "voice_XX/p_speed.jsonf";
    int speed_key_bytes = strlen(speed_key) + 1;
    char vol_key[] = "voice_XX/p_volume.jsonf";
    int vol_key_bytes = strlen(vol_key) + 1;
    for (int vi = 0; vi < CHORUS_VOICES_MAX; ++vi)
    {
        snprintf(delay_key, delay_key_bytes, "voice_%02x/p_delay.jsonf", vi);
        DSP_chorus_update_key(device, delay_key);
        snprintf(range_key, range_key_bytes, "voice_%02x/p_range.jsonf", vi);
        DSP_chorus_update_key(device, range_key);
        snprintf(speed_key, speed_key_bytes, "voice_%02x/p_speed.jsonf", vi);
        DSP_chorus_update_key(device, speed_key);
        snprintf(vol_key, vol_key_bytes, "voice_%02x/p_volume.jsonf", vi);
        DSP_chorus_update_key(device, vol_key);
    }
    return true;
}


static bool DSP_chorus_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);
    DSP_chorus* chorus = (DSP_chorus*)device;
    Device_params* params = chorus->parent.conf->params;
    int vi = -1;
    if ((vi = string_extract_index(key, "voice_", 2, "/p_delay.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* delay = Device_params_get_float(params, key);
        if (delay != NULL && *delay >= 0 && *delay < CHORUS_BUF_TIME / 2)
        {
            chorus->voices[vi].delay = chorus->voices[vi].init_delay = *delay;
            double buf_pos = chorus->voices[vi].delay *
                                 Device_get_mix_rate(device);
            assert(buf_pos >= 0);
            uint32_t buf_size = Audio_buffer_get_size(chorus->buf);
            assert(buf_pos < buf_size - 1);
            chorus->voices[vi].buf_pos = fmod((buf_size - buf_pos), buf_size);
            assert(chorus->voices[vi].buf_pos >= 0);
        }
        else
        {
            chorus->voices[vi].delay = chorus->voices[vi].init_delay = -1;
        }
    }
    else if ((vi = string_extract_index(key, "voice_", 2,
                                        "/p_range.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* range = Device_params_get_float(params, key);
        if (range != NULL && *range >= 0 && *range < CHORUS_BUF_TIME / 2)
        {
            chorus->voices[vi].range = chorus->voices[vi].init_range = *range;
            chorus->voices[vi].range_changed = true;
        }
        else
        {
            chorus->voices[vi].range = chorus->voices[vi].init_range = 0;
            chorus->voices[vi].range_changed = true;
        }
    }
    else if ((vi = string_extract_index(key, "voice_", 2,
                                        "/p_speed.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* speed = Device_params_get_float(params, key);
        if (speed != NULL && *speed >= 0)
        {
            chorus->voices[vi].speed = chorus->voices[vi].init_speed = *speed;
            chorus->voices[vi].speed_changed = true;
        }
    }
    else if ((vi = string_extract_index(key, "voice_", 2,
                                        "/p_volume.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* volume_dB = Device_params_get_float(params, key);
        if (volume_dB != NULL && *volume_dB <= DB_MAX)
        {
            chorus->voices[vi].scale =
                    chorus->voices[vi].init_scale = exp2(*volume_dB / 6);
        }
        else
        {
            chorus->voices[vi].scale = chorus->voices[vi].init_scale = 1;
        }
    }
    return true;
}


static bool DSP_chorus_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);
    DSP_chorus* chorus = (DSP_chorus*)device;
    long buf_len = CHORUS_BUF_TIME * mix_rate + 1;
    if (chorus->buf == NULL)
    {
        chorus->buf = new_Audio_buffer(buf_len);
        if (chorus->buf == NULL)
        {
            return false;
        }
    }
    else if (!Audio_buffer_resize(chorus->buf, buf_len))
    {
        return false;
    }
    Audio_buffer_clear(chorus->buf, 0, Audio_buffer_get_size(chorus->buf));
    chorus->buf_pos = 0;
    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &chorus->voices[i];
        LFO_set_mix_rate(&voice->delay_variance, mix_rate);
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
        {
            continue;
        }
        double buf_pos = voice->delay * mix_rate;
        assert(buf_pos >= 0);
        assert(buf_pos < buf_len - 1);
        voice->buf_pos = fmod((buf_len - buf_pos), buf_len);
        assert(voice->buf_pos >= 0);
    }
    return true;
}


static void DSP_chorus_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    (void)freq;
    (void)tempo;
    DSP_chorus* chorus = (DSP_chorus*)device;
    assert(string_eq(chorus->parent.type, "chorus"));
    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(device, 0, in_data);
    DSP_get_raw_output(device, 0, out_data);
    kqt_frame* buf[] = { Audio_buffer_get_buffer(chorus->buf, 0),
                         Audio_buffer_get_buffer(chorus->buf, 1) };
    int32_t buf_size = Audio_buffer_get_size(chorus->buf);
    assert(start <= until);
    DSP_chorus_check_params(chorus);
    for (uint32_t i = start; i < until; ++i)
    {
        buf[0][chorus->buf_pos] = in_data[0][i];
        buf[1][chorus->buf_pos] = in_data[1][i];
        kqt_frame val_l = 0;
        kqt_frame val_r = 0;
        for (int vi = 0; vi < CHORUS_VOICES_MAX; ++vi)
        {
            Chorus_voice* voice = &chorus->voices[vi];
            if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            {
                continue;
            }
            LFO_turn_on(&voice->delay_variance);
            voice->offset = LFO_step(&voice->delay_variance);
            assert(voice->delay + voice->offset >= 0);
            assert(voice->delay + voice->offset < CHORUS_BUF_TIME);
            double ideal_buf_pos = voice->buf_pos + freq * voice->offset;
            int32_t buf_pos = (int32_t)ideal_buf_pos;
            double remainder = ideal_buf_pos - buf_pos;
            if (buf_pos >= buf_size)
            {
                buf_pos -= buf_size;
                assert(buf_pos < buf_size);
            }
            else if (buf_pos < 0)
            {
                buf_pos += buf_size;
                assert(buf_pos >= 0);
            }
            int32_t next_pos = buf_pos + 1;
            if (next_pos >= buf_size)
            {
                next_pos = 0;
            }
            val_l += (1 - remainder) * voice->scale * buf[0][buf_pos];
            val_l += remainder * voice->scale * buf[0][next_pos];
            val_r += (1 - remainder) * voice->scale * buf[1][buf_pos];
            val_r += remainder * voice->scale * buf[1][next_pos];
            ++voice->buf_pos;
            if (voice->buf_pos >= buf_size)
            {
                assert(voice->buf_pos == buf_size);
                voice->buf_pos = 0;
            }
        }
        out_data[0][i] += val_l;
        out_data[1][i] += val_r;
        ++chorus->buf_pos;
        if (chorus->buf_pos >= buf_size)
        {
            assert(chorus->buf_pos == buf_size);
            chorus->buf_pos = 0;
        }
    }
}


static void DSP_chorus_check_params(DSP_chorus* chorus)
{
    assert(chorus != NULL);
    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &chorus->voices[i];
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
        {
            continue;
        }
        if (voice->range_changed)
        {
            voice->range_changed = false;
            if (voice->range >= voice->delay)
            {
                voice->range = 0.999 * voice->delay;
            }
            LFO_set_depth(&voice->delay_variance, voice->range);
        }
        if (voice->speed_changed)
        {
            voice->speed_changed = false;
            LFO_set_speed(&voice->delay_variance, voice->speed);
        }
    }
    return;
}


static void del_DSP_chorus(DSP* dsp)
{
    if (dsp == NULL)
    {
        return;
    }
    assert(string_eq(dsp->type, "chorus"));
    DSP_chorus* chorus = (DSP_chorus*)dsp;
    del_Audio_buffer(chorus->buf);
    memory_free(chorus);
    return;
}


