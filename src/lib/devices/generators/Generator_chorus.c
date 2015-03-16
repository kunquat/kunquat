

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
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
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/Generator.h>
#include <devices/dsps/DSP_common.h>
#include <devices/generators/Generator_chorus.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/LFO.h>
#include <string/common.h>


#define CHORUS_BUF_TIME 0.25
#define CHORUS_VOICES_MAX 32
#define DB_MAX 18


typedef struct Chorus_voice_params
{
    double delay;
    double range;
    double speed;
    double volume;
} Chorus_voice_params;


typedef struct Chorus_voice
{
    double delay;
    double offset;
    LFO delay_variance;
    double range;
    double speed;
    double volume;
    int32_t buf_pos;
} Chorus_voice;


typedef struct Chorus_state
{
    Gen_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
    Chorus_voice voices[CHORUS_VOICES_MAX];
} Chorus_state;


static void Chorus_voice_reset(
        Chorus_voice* voice,
        const Chorus_voice_params* params,
        int32_t audio_rate,
        uint32_t buffer_size)
{
    voice->delay = 0;
    voice->offset = 0;
    voice->range = 0;
    voice->speed = 0;
    voice->volume = 0;
    voice->buf_pos = 0;

    voice->delay = params->delay;
    if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
        return;

    voice->offset = 0;
    voice->delay = params->delay;

    voice->range = params->range;
    if (voice->range >= voice->delay)
        voice->range = 0.999 * voice->delay;
    LFO_set_depth(&voice->delay_variance, voice->range);

    voice->speed = params->speed;
    LFO_set_speed(&voice->delay_variance, voice->speed);

    voice->volume = params->volume;

    double buf_pos = voice->delay * audio_rate;
    assert(buf_pos >= 0);
    assert(buf_pos < buffer_size - 1);
    voice->buf_pos = fmod((buffer_size - buf_pos), buffer_size);
    assert(voice->buf_pos >= 0);

    return;
}


static void Chorus_state_reset(
        Chorus_state* cstate,
        const Chorus_voice_params voice_params[CHORUS_VOICES_MAX])
{
    assert(cstate != NULL);
    assert(voice_params != NULL);

    Gen_state_reset(&cstate->parent);

    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);
    Audio_buffer_clear(cstate->buf, 0, buf_size);
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        const Chorus_voice_params* params = &voice_params[i];
        Chorus_voice* voice = &cstate->voices[i];
        Chorus_voice_reset(
                voice, params, cstate->parent.parent.audio_rate, buf_size);
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


typedef struct Generator_chorus
{
    Device_impl parent;

    Chorus_voice_params voice_params[CHORUS_VOICES_MAX];
} Generator_chorus;


static bool Generator_chorus_init(Device_impl* dimpl);

static Device_state* Generator_chorus_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Generator_chorus_update_tempo(
        const Device_impl* dimpl, Device_state* dstate, double tempo);

static void Generator_chorus_reset(const Device_impl* dimpl, Device_state* dstate);


#define CHORUS_PARAM(name, dev_key, update_key, def_value)                 \
    static Set_float_func Generator_chorus_set_voice_ ## name;             \
    static Set_state_float_func Generator_chorus_set_state_voice_ ## name; \
    static Update_float_func Generator_chorus_update_state_voice_ ## name;
#include <devices/generators/Generator_chorus_params.h>


static void Generator_chorus_clear_history(
        const Device_impl* dimpl, Gen_state* gen_state);

static bool Generator_chorus_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate);

static void Generator_chorus_process(
        const Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);

static void del_Generator_chorus(Device_impl* dsp_impl);


Device_impl* new_Generator_chorus(Generator* gen)
{
    Generator_chorus* chorus = memory_alloc_item(Generator_chorus);
    if (chorus == NULL)
        return NULL;

    chorus->parent.device = (Device*)gen;

    Device_impl_register_init(&chorus->parent, Generator_chorus_init);
    Device_impl_register_destroy(&chorus->parent, del_Generator_chorus);

    return &chorus->parent;
}


static bool Generator_chorus_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Generator_chorus* chorus = (Generator_chorus*)dimpl;

    Device_set_process(chorus->parent.device, Generator_chorus_process);

    Device_set_state_creator(chorus->parent.device, Generator_chorus_create_state);

    Generator_set_clear_history((Generator*)chorus->parent.device, Generator_chorus_clear_history);

    Device_impl_register_update_tempo(&chorus->parent, Generator_chorus_update_tempo);
    Device_impl_register_reset_device_state(&chorus->parent, Generator_chorus_reset);

    // Register key set/update handlers
    bool reg_success = true;

#define CHORUS_PARAM(name, dev_key, update_key, def_value) \
    reg_success &= Device_impl_register_set_float(         \
            &chorus->parent,                               \
            dev_key,                                       \
            def_value,                                     \
            Generator_chorus_set_voice_ ## name,           \
            Generator_chorus_set_state_voice_ ## name);
#include <devices/generators/Generator_chorus_params.h>

#define CHORUS_PARAM(name, dev_key, update_key, def_value)  \
    reg_success &= Device_impl_register_update_state_float( \
            &chorus->parent,                                \
            update_key,                                     \
            Generator_chorus_update_state_voice_ ## name);
#include <devices/generators/Generator_chorus_params.h>

    if (!reg_success)
        return false;

    Device_impl_register_set_audio_rate(
            &chorus->parent, Generator_chorus_set_audio_rate);

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice_params* params = &chorus->voice_params[i];

        params->delay = -1;
        params->range = 0;
        params->speed = 0;
        params->volume = 1;
    }

    return true;
}


static Device_state* Generator_chorus_create_state(
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

    Gen_state_init(&cstate->parent, device, audio_rate, audio_buffer_size);
    cstate->parent.parent.destroy = del_Chorus_state;
    cstate->buf = NULL;
    cstate->buf_pos = 0;

    const long buf_len = CHORUS_BUF_TIME * audio_rate + 1;
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

    Generator_chorus* chorus = (Generator_chorus*)device->dimpl;
    Chorus_state_reset(cstate, chorus->voice_params);

    return &cstate->parent.parent;
}


static void Generator_chorus_update_tempo(
        const Device_impl* dimpl, Device_state* dstate, double tempo)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);
    (void)dimpl;

    Chorus_state* cstate = (Chorus_state*)dstate;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        LFO_set_tempo(&voice->delay_variance, tempo);
    }

    return;
}


static void Generator_chorus_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    const Generator_chorus* chorus = (const Generator_chorus*)dimpl;
    Chorus_state* cstate = (Chorus_state*)dstate;

    Generator_chorus_clear_history(dimpl, &cstate->parent); // XXX: do we need this?

    Chorus_state_reset(cstate, chorus->voice_params);

    return;
}


static void Generator_chorus_clear_history(
        const Device_impl* dimpl, Gen_state* gen_state)
{
    assert(dimpl != NULL);
    //assert(string_eq(dsp->type, "chorus"));
    assert(gen_state != NULL);
    (void)dimpl;

    Chorus_state* cstate = (Chorus_state*)gen_state;
    Audio_buffer_clear(cstate->buf, 0, cstate->parent.parent.audio_buffer_size);

    return;
}


static double get_voice_delay(double value)
{
    return (value >= 0 && value < CHORUS_BUF_TIME / 2) ? value : -1.0;
}


static double get_voice_range(double value)
{
    return (value >= 0 && value < CHORUS_BUF_TIME / 2) ? value : 0.0;
}


static double get_voice_speed(double value)
{
    return (value >= 0) ? value : 0.0;
}


static double get_voice_volume(double value)
{
    return (value <= DB_MAX) ? exp2(value / 6.0) : 1.0;
}


#define CHORUS_PARAM(name, dev_key, update_key, def_value)               \
    static bool Generator_chorus_set_voice_ ## name(                     \
            Device_impl* dimpl, Key_indices indices, double value)       \
    {                                                                    \
        assert(dimpl != NULL);                                           \
        assert(indices != NULL);                                         \
                                                                         \
        if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)           \
            return true;                                                 \
                                                                         \
        Generator_chorus* chorus = (Generator_chorus*)dimpl;             \
        Chorus_voice_params* params = &chorus->voice_params[indices[0]]; \
                                                                         \
        params->name = get_voice_ ## name(value);                        \
                                                                         \
        return true;                                                     \
    }
#include <devices/generators/Generator_chorus_params.h>


#define CHORUS_PARAM(name, dev_key, update_key, def_value)                           \
    static bool Generator_chorus_set_state_voice_ ## name(                           \
            const Device_impl* dimpl,                                                \
            Device_state* dstate,                                                    \
            Key_indices indices,                                                     \
            double value)                                                            \
    {                                                                                \
        assert(dimpl != NULL);                                                       \
        assert(dstate != NULL);                                                      \
        assert(indices != NULL);                                                     \
                                                                                     \
        Generator_chorus_update_state_voice_ ## name(dimpl, dstate, indices, value); \
                                                                                     \
        return true;                                                                 \
    }
#include <devices/generators/Generator_chorus_params.h>


static void Generator_chorus_update_state_voice_delay(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return;

    const Generator_chorus* chorus = (const Generator_chorus*)dimpl;
    const Chorus_voice_params* params = &chorus->voice_params[indices[0]];

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);

    voice->delay = get_voice_delay(value);

    Chorus_voice_reset(
            voice, params, cstate->parent.parent.audio_rate, buf_size);

    return;
}


static void Generator_chorus_update_state_voice_range(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return;

    const Generator_chorus* chorus = (const Generator_chorus*)dimpl;
    const Chorus_voice_params* params = &chorus->voice_params[indices[0]];

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);

    voice->range = get_voice_range(value);

    if (voice->range >= voice->delay)
        voice->range = 0.999 * voice->delay;

    LFO_set_depth(&voice->delay_variance, voice->range);

    Chorus_voice_reset(
            voice, params, cstate->parent.parent.audio_rate, buf_size);

    return;
}


static void Generator_chorus_update_state_voice_speed(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return;

    const Generator_chorus* chorus = (const Generator_chorus*)dimpl;
    const Chorus_voice_params* params = &chorus->voice_params[indices[0]];

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);

    voice->speed = get_voice_speed(value);

    LFO_set_speed(&voice->delay_variance, voice->speed);

    Chorus_voice_reset(
            voice, params, cstate->parent.parent.audio_rate, buf_size);

    return;
}


static void Generator_chorus_update_state_voice_volume(
        const Device_impl* dimpl,
        Device_state* dstate,
        Key_indices indices,
        double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(indices != NULL);

    if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)
        return;

    const Generator_chorus* chorus = (const Generator_chorus*)dimpl;
    const Chorus_voice_params* params = &chorus->voice_params[indices[0]];

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    uint32_t buf_size = Audio_buffer_get_size(cstate->buf);

    voice->volume = get_voice_volume(value);

    Chorus_voice_reset(
            voice, params, cstate->parent.parent.audio_rate, buf_size);

    return;
}


static bool Generator_chorus_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);
    (void)dimpl;

    long buf_len = CHORUS_BUF_TIME * audio_rate + 1;

    Chorus_state* cstate = (Chorus_state*)dstate;

    assert(cstate->buf != NULL);
    if (!Audio_buffer_resize(cstate->buf, buf_len))
        return false;

    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];

        LFO_set_mix_rate(&voice->delay_variance, audio_rate);
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            continue;

        double buf_pos = voice->delay * audio_rate;
        assert(buf_pos >= 0);
        assert(buf_pos < buf_len - 1);
        voice->buf_pos = fmod((buf_len - buf_pos), buf_len);
        assert(voice->buf_pos >= 0);
    }

    return true;
}


// FIXME: get rid of this mess
static void check_params(Chorus_state* cstate, double tempo)
{
    assert(cstate != NULL);
    assert(tempo > 0);

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
            continue;

        LFO_set_tempo(&voice->delay_variance, tempo);
    }

    return;
}


static void Generator_chorus_process(
        const Device* device,
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

    check_params(cstate, tempo);

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

            val_l += (1 - remainder) * voice->volume * buf[0][buf_pos];
            val_l += remainder * voice->volume * buf[0][next_pos];
            val_r += (1 - remainder) * voice->volume * buf[1][buf_pos];
            val_r += remainder * voice->volume * buf[1][next_pos];

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


static void del_Generator_chorus(Device_impl* gen_impl)
{
    if (gen_impl == NULL)
        return;

    //assert(string_eq(dsp->type, "chorus"));
    Generator_chorus* chorus = (Generator_chorus*)gen_impl;
    memory_free(chorus);

    return;
}


