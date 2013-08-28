

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
#include <Device_impl.h>
#include <DSP.h>
#include <DSP_common.h>
#include <DSP_chorus.h>
#include <math_common.h>
#include <memory.h>
#include <player/LFO.h>
#include <string_common.h>
#include <xassert.h>


#define CHORUS_BUF_TIME 0.25
#define CHORUS_VOICES_MAX 32
#define DB_MAX 18


typedef struct Chorus_voice_params
{
    double delay;
    double range;
    double speed;
    double scale;
} Chorus_voice_params;


typedef struct Chorus_voice
{
    double delay;
    double offset;
    LFO delay_variance;
    double range;
    bool range_changed;
    double speed;
    bool speed_changed;
    double scale;
    int32_t buf_pos;
} Chorus_voice;


typedef struct Chorus_state
{
    DSP_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
    Chorus_voice voices[CHORUS_VOICES_MAX];
} Chorus_state;


static void Chorus_state_reset(
        Chorus_state* cstate,
        const Chorus_voice_params voice_params[CHORUS_VOICES_MAX])
{
    assert(cstate != NULL);
    assert(voice_params != NULL);

    DSP_state_reset(&cstate->parent);

    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);
    Audio_buffer_clear(cstate->buf, 0, buf_size);
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        const Chorus_voice_params* params = &voice_params[i];
        Chorus_voice* voice = &cstate->voices[i];
        voice->delay = params->delay;
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            continue;

        voice->offset = 0;
        voice->delay = params->delay;
        voice->range_changed |= (voice->range != params->range);
        voice->range = params->range;
        voice->speed_changed |= (voice->speed != params->speed);
        voice->speed = params->speed;
        voice->scale = params->scale;

        double buf_pos = voice->delay * cstate->parent.parent.audio_rate;
        assert(buf_pos >= 0);
        assert(buf_pos < buf_size - 1);
        voice->buf_pos = fmod((buf_size - buf_pos), buf_size);
        assert(voice->buf_pos >= 0);
    }

    return;
}


static void del_Chorus_state(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Chorus_state* cstate = (Chorus_state*)dev_state;
    if (cstate->buf != NULL)
    {
        del_Audio_buffer(cstate->buf);
        cstate->buf = NULL;
    }

    return;
}


typedef struct DSP_chorus
{
    Device_impl parent;

    Chorus_voice_params voice_params[CHORUS_VOICES_MAX];
} DSP_chorus;


static Device_state* DSP_chorus_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void DSP_chorus_reset(const Device_impl* dimpl, Device_state* dstate);

#if 0
static bool DSP_chorus_update_state_voice_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);
static bool DSP_chorus_update_state_voice_range(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);
static bool DSP_chorus_update_state_voice_speed(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);
static bool DSP_chorus_update_state_voice_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value);
#endif

static void DSP_chorus_clear_history(DSP* dsp, DSP_state* dsp_state);
static bool DSP_chorus_sync(Device* device, Device_states* dstates);
static bool DSP_chorus_update_key(Device* device, const char* key);
static bool DSP_chorus_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate);

static void DSP_chorus_check_params(
        DSP_chorus* chorus,
        Chorus_state* cstate,
        int32_t audio_rate,
        double tempo);

static void DSP_chorus_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_DSP_chorus(Device_impl* dsp_impl);


Device_impl* new_DSP_chorus(DSP* dsp, uint32_t buffer_size, uint32_t mix_rate)
{
    assert(buffer_size > 0);
    assert(buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(mix_rate > 0);

    DSP_chorus* chorus = memory_alloc_item(DSP_chorus);
    if (chorus == NULL)
        return NULL;

    if (!Device_impl_init(&chorus->parent, del_DSP_chorus, mix_rate, buffer_size))
    {
        memory_free(chorus);
        return NULL;
    }

    chorus->parent.device = (Device*)dsp;

    Device_set_process((Device*)dsp, DSP_chorus_process);
#if 0
    if (!DSP_init(&chorus->parent, del_DSP_chorus,
                  DSP_chorus_process, buffer_size, mix_rate))
    {
        memory_free(chorus);
        return NULL;
    }
#endif

    Device_set_state_creator(chorus->parent.device, DSP_chorus_create_state);

    DSP_set_clear_history((DSP*)chorus->parent.device, DSP_chorus_clear_history);

    Device_impl_register_reset_device_state(&chorus->parent, DSP_chorus_reset);

    // State update functions, TODO: enable once implemented properly
#if 0
    Device_impl_register_update_state_float(
            &chorus->parent,
            "v_XX/d",
            DSP_chorus_update_state_voice_delay);

    Device_impl_register_update_state_float(
            &chorus->parent,
            "v_XX/r",
            DSP_chorus_update_state_voice_range);

    Device_impl_register_update_state_float(
            &chorus->parent,
            "v_XX/s",
            DSP_chorus_update_state_voice_speed);

    Device_impl_register_update_state_float(
            &chorus->parent,
            "v_XX/v",
            DSP_chorus_update_state_voice_volume);
#endif

    Device_set_mix_rate_changer(
            chorus->parent.device,
            DSP_chorus_set_mix_rate);
    Device_set_sync(chorus->parent.device, DSP_chorus_sync);
    Device_set_update_key(chorus->parent.device, DSP_chorus_update_key);

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice_params* params = &chorus->voice_params[i];

        params->delay = -1;
        params->range = 0;
        params->speed = 0;
        params->scale = 1;
    }

    Device_register_port(chorus->parent.device, DEVICE_PORT_TYPE_RECEIVE, 0);
    Device_register_port(chorus->parent.device, DEVICE_PORT_TYPE_SEND, 0);

#if 0
    if (!DSP_chorus_set_mix_rate(&chorus->parent.parent, mix_rate))
    {
        del_DSP(&chorus->parent);
        return NULL;
    }
#endif

    return &chorus->parent;
}


static Device_state* DSP_chorus_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Chorus_state* cstate = memory_alloc_item(Chorus_state);
    if (cstate == NULL)
        return NULL;

    DSP_state_init(&cstate->parent, device, audio_rate, audio_buffer_size);
    cstate->parent.parent.destroy = del_Chorus_state;
    cstate->buf = NULL;
    cstate->buf_pos = 0;

    const long buf_len = CHORUS_BUF_TIME * audio_buffer_size + 1;
    cstate->buf = new_Audio_buffer(buf_len);
    if (cstate->buf == NULL)
    {
        del_Device_state(&cstate->parent.parent);
        return NULL;
    }

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        LFO_init(&voice->delay_variance, LFO_MODE_LINEAR);
    }

    DSP_chorus* chorus = (DSP_chorus*)device->dimpl;
    Chorus_state_reset(cstate, chorus->voice_params);

    return &cstate->parent.parent;
}


static void DSP_chorus_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    DSP_chorus* chorus = (DSP_chorus*)dimpl;
    Chorus_state* cstate = (Chorus_state*)dstate;

    DSP_chorus_clear_history((DSP*)chorus->parent.device, &cstate->parent); // XXX: do we need this?

    Chorus_state_reset(cstate, chorus->voice_params);

    return;
}


static void DSP_chorus_clear_history(DSP* dsp, DSP_state* dsp_state)
{
    assert(dsp != NULL);
    //assert(string_eq(dsp->type, "chorus"));
    assert(dsp_state != NULL);

    Chorus_state* cstate = (Chorus_state*)dsp_state;
    Audio_buffer_clear(cstate->buf, 0, cstate->parent.parent.audio_buffer_size);

    return;
}


static bool DSP_chorus_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

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


#if 0
static bool DSP_chorus_update_state_voice_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return false;

    Chorus_state* cstate = (Chorus_state*)dstate;

    cstate->voices[indices[0]].delay =
        (value >= 0 && value < CHORUS_BUF_TIME / 2) ? value : -1;

    return true;
}


static bool DSP_chorus_update_state_voice_range(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return false;

    Chorus_state* cstate = (Chorus_state*)dstate;

    cstate->voices[indices[0]].range =
        (value >= 0 && value < CHORUS_BUF_TIME / 2) ? value : 0;

    return true;
}


static bool DSP_chorus_update_state_voice_speed(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return false;

    Chorus_state* cstate = (Chorus_state*)dstate;

    cstate->voices[indices[0]].speed = (value >= 0) ? value : 0;

    return true;
}


static bool DSP_chorus_update_state_voice_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t indices[DEVICE_KEY_INDICES_MAX],
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return false;

    Chorus_state* cstate = (Chorus_state*)dstate;

    cstate->voices[indices[0]].scale =
        (value <= DB_MAX) ? exp2(value / 6) : 1;

    return true;
}
#endif


static bool DSP_chorus_update_key(Device* device, const char* key)
{
    assert(device != NULL);
    assert(key != NULL);

    DSP_chorus* chorus = (DSP_chorus*)device->dimpl;
    Device_params* params = ((DSP*)chorus->parent.device)->conf->params;

    int vi = -1;
    if ((vi = string_extract_index(key, "voice_", 2, "/p_delay.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* delay = Device_params_get_float(params, key);

        Chorus_voice_params* params = &chorus->voice_params[vi];

        if (delay != NULL && *delay >= 0 && *delay < CHORUS_BUF_TIME / 2)
        {
            params->delay = *delay;
#if 0
            double buf_pos = voice->delay * Device_get_mix_rate(device);
            assert(buf_pos >= 0);
            uint32_t buf_size = Audio_buffer_get_size(chorus->buf);
            assert(buf_pos < buf_size - 1);
            voice->buf_pos = fmod((buf_size - buf_pos), buf_size);
            assert(voice->buf_pos >= 0);
#endif
        }
        else
        {
            params->delay = -1;
        }
    }
    else if ((vi = string_extract_index(
                    key, "voice_", 2, "/p_range.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* range = Device_params_get_float(params, key);

        Chorus_voice_params* params = &chorus->voice_params[vi];

        if (range != NULL && *range >= 0 && *range < CHORUS_BUF_TIME / 2)
            params->range = *range;
        else
            params->range = 0;
    }
    else if ((vi = string_extract_index(
                    key, "voice_", 2, "/p_speed.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* speed = Device_params_get_float(params, key);

        Chorus_voice_params* params = &chorus->voice_params[vi];

        if (speed != NULL && *speed >= 0)
            params->speed = *speed;
        else
            params->speed = 0;
    }
    else if ((vi = string_extract_index(
                    key, "voice_", 2, "/p_volume.jsonf")) >= 0
            && vi < CHORUS_VOICES_MAX)
    {
        double* volume_dB = Device_params_get_float(params, key);

        Chorus_voice_params* params = &chorus->voice_params[vi];

        if (volume_dB != NULL && *volume_dB <= DB_MAX)
            params->scale = exp2(*volume_dB / 6);
        else
            params->scale = 1;
    }

    return true;
}


static bool DSP_chorus_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(mix_rate > 0);

    long buf_len = CHORUS_BUF_TIME * mix_rate + 1;

    Chorus_state* cstate = (Chorus_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));

    assert(cstate->buf != NULL);
    if (!Audio_buffer_resize(cstate->buf, buf_len))
        return false;

    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];

        LFO_set_mix_rate(&voice->delay_variance, mix_rate);
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            continue;

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

    Chorus_state* cstate = (Chorus_state*)Device_states_get_state(
            states,
            Device_get_id(device));
    assert(cstate != NULL);

    DSP_chorus* chorus = (DSP_chorus*)device->dimpl;
    //assert(string_eq(chorus->parent.type, "chorus"));

    kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    DSP_get_raw_input(&cstate->parent.parent, 0, in_data);
    DSP_get_raw_output(&cstate->parent.parent, 0, out_data);

    kqt_frame* buf[] =
    {
        Audio_buffer_get_buffer(cstate->buf, 0),
        Audio_buffer_get_buffer(cstate->buf, 1),
    };

    const int32_t buf_size = Audio_buffer_get_size(cstate->buf);
    assert(start <= until);

    DSP_chorus_check_params(chorus, cstate, freq, tempo);

    for (uint32_t i = start; i < until; ++i)
    {
        buf[0][cstate->buf_pos] = in_data[0][i];
        buf[1][cstate->buf_pos] = in_data[1][i];

        kqt_frame val_l = 0;
        kqt_frame val_r = 0;

        for (int vi = 0; vi < CHORUS_VOICES_MAX; ++vi)
        {
            Chorus_voice* voice = &cstate->voices[vi];
            if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
                continue;

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
                next_pos = 0;

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

        ++cstate->buf_pos;
        if (cstate->buf_pos >= buf_size)
        {
            assert(cstate->buf_pos == buf_size);
            cstate->buf_pos = 0;
        }
    }

    return;
}


static void DSP_chorus_check_params(
        DSP_chorus* chorus,
        Chorus_state* cstate,
        int32_t audio_rate,
        double tempo)
{
    assert(chorus != NULL);
    assert(cstate != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            continue;

        LFO_set_mix_rate(&voice->delay_variance, audio_rate);
        LFO_set_tempo(&voice->delay_variance, tempo);

        if (voice->range_changed)
        {
            voice->range_changed = false;
            if (voice->range >= voice->delay)
                voice->range = 0.999 * voice->delay;

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


static void del_DSP_chorus(Device_impl* dsp_impl)
{
    if (dsp_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "chorus"));
    DSP_chorus* chorus = (DSP_chorus*)dsp_impl;
    memory_free(chorus);

    return;
}


