

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
#include <devices/Processor.h>
#include <devices/processors/Proc_chorus.h>
#include <devices/processors/Proc_utils.h>
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
    LFO delay_variance;
    double range;
    double speed;
    double volume;
} Chorus_voice;


typedef struct Chorus_state
{
    Proc_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
    Chorus_voice voices[CHORUS_VOICES_MAX];
} Chorus_state;


static void Chorus_voice_reset(
        Chorus_voice* voice, const Chorus_voice_params* params, int32_t audio_rate)
{
    assert(voice != NULL);
    assert(params != NULL);
    ignore(audio_rate);

    voice->delay = 0;
    voice->range = 0;
    voice->speed = 0;
    voice->volume = 0;

    voice->delay = params->delay;
    if (voice->delay < 0 || voice->delay >= CHORUS_BUF_TIME / 2)
        return;

    voice->delay = params->delay;

    voice->range = min(params->range, 0.999 * voice->delay);
    LFO_set_depth(&voice->delay_variance, voice->range);

    voice->speed = params->speed;
    LFO_set_speed(&voice->delay_variance, voice->speed);

    voice->volume = params->volume;

    return;
}


static void Chorus_state_reset(
        Chorus_state* cstate,
        const Chorus_voice_params voice_params[CHORUS_VOICES_MAX])
{
    assert(cstate != NULL);
    assert(voice_params != NULL);

    Proc_state_reset(&cstate->parent);

    const uint32_t delay_buf_size = Audio_buffer_get_size(cstate->buf);
    Audio_buffer_clear(cstate->buf, 0, delay_buf_size);
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        const Chorus_voice_params* params = &voice_params[i];
        Chorus_voice* voice = &cstate->voices[i];
        Chorus_voice_reset(voice, params, cstate->parent.parent.audio_rate);
    }

    return;
}


static void Chorus_state_deinit(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Chorus_state* cstate = (Chorus_state*)dev_state;
    if (cstate->buf != NULL)
    {
        del_Audio_buffer(cstate->buf);
        cstate->buf = NULL;
    }

    Proc_state_deinit(&cstate->parent.parent);

    return;
}


typedef struct Proc_chorus
{
    Device_impl parent;

    Chorus_voice_params voice_params[CHORUS_VOICES_MAX];
} Proc_chorus;


static bool Proc_chorus_init(Device_impl* dimpl);

static Device_state* Proc_chorus_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

static void Proc_chorus_reset(const Device_impl* dimpl, Device_state* dstate);


#define CHORUS_PARAM(name, dev_key, update_key, def_value)            \
    static Set_float_func Proc_chorus_set_voice_ ## name;             \
    static Set_state_float_func Proc_chorus_set_state_voice_ ## name; \
    static Set_cv_float_func Proc_chorus_set_cv_voice_ ## name;
#include <devices/processors/Proc_chorus_params.h>


static void Proc_chorus_clear_history(const Device_impl* dimpl, Proc_state* proc_state);

static bool Proc_chorus_set_audio_rate(
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate);

static void Proc_chorus_process(
        const Device* device,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);

static void del_Proc_chorus(Device_impl* dimpl);


Device_impl* new_Proc_chorus(Processor* proc)
{
    Proc_chorus* chorus = memory_alloc_item(Proc_chorus);
    if (chorus == NULL)
        return NULL;

    chorus->parent.device = (Device*)proc;

    Device_impl_register_init(&chorus->parent, Proc_chorus_init);
    Device_impl_register_destroy(&chorus->parent, del_Proc_chorus);

    return &chorus->parent;
}


static bool Proc_chorus_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_chorus* chorus = (Proc_chorus*)dimpl;

    Device_set_process(chorus->parent.device, Proc_chorus_process);

    Device_set_state_creator(chorus->parent.device, Proc_chorus_create_state);

    Processor_set_clear_history(
            (Processor*)chorus->parent.device, Proc_chorus_clear_history);

    Device_impl_register_reset_device_state(&chorus->parent, Proc_chorus_reset);

    // Register key set/update handlers
    bool reg_success = true;

#define CHORUS_PARAM(name, dev_key, update_key, def_value) \
    reg_success &= Device_impl_register_set_float(         \
            &chorus->parent,                               \
            dev_key,                                       \
            def_value,                                     \
            Proc_chorus_set_voice_ ## name,                \
            Proc_chorus_set_state_voice_ ## name);
#include <devices/processors/Proc_chorus_params.h>

#define CHORUS_PARAM(name, dev_key, update_key, def_value)  \
    reg_success &= Device_impl_register_updaters_cv_float(  \
            &chorus->parent,                                \
            update_key,                                     \
            Proc_chorus_set_cv_voice_ ## name,              \
            NULL, NULL, NULL, NULL, NULL, NULL);
#include <devices/processors/Proc_chorus_params.h>

    // TODO: register appropriate sliders and oscillators

    if (!reg_success)
        return false;

    Device_impl_register_set_audio_rate(
            &chorus->parent, Proc_chorus_set_audio_rate);

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


static Device_state* Proc_chorus_create_state(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Chorus_state* cstate = memory_alloc_item(Chorus_state);
    if (cstate == NULL)
        return NULL;

    if (!Proc_state_init(&cstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(cstate);
        return NULL;
    }

    cstate->parent.parent.deinit = Chorus_state_deinit;
    cstate->buf = NULL;
    cstate->buf_pos = 0;

    const int32_t delay_buf_size = CHORUS_BUF_TIME * audio_rate + 1;

    cstate->buf = new_Audio_buffer(delay_buf_size);
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

    Proc_chorus* chorus = (Proc_chorus*)device->dimpl;
    Chorus_state_reset(cstate, chorus->voice_params);

    return &cstate->parent.parent;
}


static void Proc_chorus_reset(const Device_impl* dimpl, Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    const Proc_chorus* chorus = (const Proc_chorus*)dimpl;
    Chorus_state* cstate = (Chorus_state*)dstate;

    Proc_chorus_clear_history(dimpl, &cstate->parent); // XXX: do we need this?

    Chorus_state_reset(cstate, chorus->voice_params);

    return;
}


static void Proc_chorus_clear_history(const Device_impl* dimpl, Proc_state* proc_state)
{
    assert(dimpl != NULL);
    assert(proc_state != NULL);

    Chorus_state* cstate = (Chorus_state*)proc_state;
    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));

    cstate->buf_pos = 0;

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
    static bool Proc_chorus_set_voice_ ## name(                          \
            Device_impl* dimpl, Key_indices indices, double value)       \
    {                                                                    \
        assert(dimpl != NULL);                                           \
        assert(indices != NULL);                                         \
                                                                         \
        if (indices[0] < 0 || indices[0] >= CHORUS_VOICES_MAX)           \
            return true;                                                 \
                                                                         \
        Proc_chorus* chorus = (Proc_chorus*)dimpl;                       \
        Chorus_voice_params* params = &chorus->voice_params[indices[0]]; \
                                                                         \
        params->name = get_voice_ ## name(value);                        \
                                                                         \
        return true;                                                     \
    }
#include <devices/processors/Proc_chorus_params.h>


#define CHORUS_PARAM(name, dev_key, update_key, def_value)                  \
    static bool Proc_chorus_set_state_voice_ ## name(                       \
            const Device_impl* dimpl,                                       \
            Device_state* dstate,                                           \
            Key_indices indices,                                            \
            double value)                                                   \
    {                                                                       \
        assert(dimpl != NULL);                                              \
        assert(dstate != NULL);                                             \
        assert(indices != NULL);                                            \
                                                                            \
        Proc_chorus_set_cv_voice_ ## name(dimpl, dstate, indices, value);   \
                                                                            \
        return true;                                                        \
    }
#include <devices/processors/Proc_chorus_params.h>


static void Proc_chorus_set_cv_voice_delay(
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

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];

    voice->delay = get_voice_delay(value);
    voice->range = min(voice->range, 0.999 * voice->delay);

    return;
}


static void Proc_chorus_set_cv_voice_range(
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

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];

    voice->range = get_voice_range(value);
    voice->range = min(voice->range, 0.999 * voice->delay);

    LFO_set_depth(&voice->delay_variance, voice->range);

    return;
}


static void Proc_chorus_set_cv_voice_speed(
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

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];

    voice->speed = get_voice_speed(value);

    LFO_set_speed(&voice->delay_variance, voice->speed);

    return;
}


static void Proc_chorus_set_cv_voice_volume(
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

    Chorus_state* cstate = (Chorus_state*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];

    voice->volume = get_voice_volume(value);

    return;
}


static bool Proc_chorus_set_audio_rate(
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    const int32_t delay_buf_size = CHORUS_BUF_TIME * audio_rate + 1;

    Chorus_state* cstate = (Chorus_state*)dstate;

    assert(cstate->buf != NULL);
    if (!Audio_buffer_resize(cstate->buf, delay_buf_size))
        return false;

    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        LFO_set_mix_rate(&voice->delay_variance, audio_rate);
    }

    return true;
}


static void Proc_chorus_process(
        const Device* device,
        Device_states* dstates,
        const Work_buffers* wbs,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(wbs != NULL);
    assert(buf_start <= buf_stop);
    assert(audio_rate > 0);
    assert(tempo > 0);

    Chorus_state* cstate = (Chorus_state*)Device_states_get_state(
            dstates, Device_get_id(device));
    assert(cstate != NULL);

    //assert(string_eq(chorus->parent.type, "chorus"));

    const kqt_frame* in_data[] = { NULL, NULL };
    kqt_frame* out_data[] = { NULL, NULL };
    get_raw_input(&cstate->parent.parent, 0, in_data);
    get_raw_output(&cstate->parent.parent, 0, out_data);

    kqt_frame* buf[] =
    {
        Audio_buffer_get_buffer(cstate->buf, 0),
        Audio_buffer_get_buffer(cstate->buf, 1),
    };

    const int32_t delay_buf_size = Audio_buffer_get_size(cstate->buf);

    static const int CHORUS_WORK_BUFFER_TOTAL_OFFSETS = WORK_BUFFER_IMPL_1;

    float* total_offsets = Work_buffers_get_buffer_contents_mut(
            wbs, CHORUS_WORK_BUFFER_TOTAL_OFFSETS);

    int32_t cur_cstate_buf_pos = cstate->buf_pos;

    // Mix chorus voices
    for (int vi = 0; vi < CHORUS_VOICES_MAX; ++vi)
    {
        Chorus_voice* voice = &cstate->voices[vi];
        if ((voice->delay < 0) || (voice->delay >= CHORUS_BUF_TIME / 2))
            continue;

        LFO_turn_on(&voice->delay_variance);

        const double voice_volume = voice->volume;

        //int32_t offset_base = voice->buf_pos;
        const double offset_base = -voice->delay * audio_rate;

        // Get total offsets
        for (uint32_t i = buf_start, chunk_offset = 0; i < buf_stop; ++i, ++chunk_offset)
        {
            const double cur_delay_var = LFO_step(&voice->delay_variance) * audio_rate;
            const double total_offset = offset_base + chunk_offset + cur_delay_var;
            total_offsets[i] = total_offset;
        }

        for (uint32_t i = buf_start; i < buf_stop; ++i)
        {
            const float total_offset = total_offsets[i];

            // Get buffer positions
            const int32_t cur_pos = (int32_t)floor(total_offset);
            const double remainder = total_offset - cur_pos;
            assert(cur_pos <= (int32_t)i);
            assert(!(cur_pos == (int32_t)i) || (remainder == 0));
            const int32_t next_pos = cur_pos + 1;

            // Get audio frames
            double cur_val_l = 0;
            double cur_val_r = 0;
            double next_val_l = 0;
            double next_val_r = 0;

            if (cur_pos >= 0)
            {
                const int32_t in_cur_pos = buf_start + cur_pos;
                assert(in_cur_pos < (int32_t)buf_stop);
                cur_val_l = in_data[0][in_cur_pos];
                cur_val_r = in_data[1][in_cur_pos];

                const int32_t in_next_pos = min(buf_start + next_pos, i);
                assert(in_next_pos < (int32_t)buf_stop);
                next_val_l = in_data[0][in_next_pos];
                next_val_r = in_data[1][in_next_pos];
            }
            else
            {
                const int32_t cur_delay_buf_pos =
                    (cur_cstate_buf_pos + cur_pos + delay_buf_size) % delay_buf_size;
                assert(cur_delay_buf_pos >= 0);

                cur_val_l = buf[0][cur_delay_buf_pos];
                cur_val_r = buf[1][cur_delay_buf_pos];

                if (next_pos < 0)
                {
                    const int32_t next_delay_buf_pos =
                        (cur_cstate_buf_pos + next_pos + delay_buf_size) %
                        delay_buf_size;
                    assert(next_delay_buf_pos >= 0);

                    next_val_l = buf[0][next_delay_buf_pos];
                    next_val_r = buf[1][next_delay_buf_pos];
                }
                else
                {
                    assert(next_pos == 0);
                    next_val_l = in_data[0][buf_start];
                    next_val_r = in_data[1][buf_start];
                }
            }

            // Create output frame
            const double prev_scale = 1 - remainder;
            const kqt_frame val_l =
                (prev_scale * voice_volume * cur_val_l) +
                (remainder * voice_volume * next_val_l);
            const kqt_frame val_r =
                (prev_scale * voice_volume * cur_val_r) +
                (remainder * voice_volume * next_val_r);

            out_data[0][i] += val_l;
            out_data[1][i] += val_r;
        }
    }

    // Update the chorus state buffers
    for (uint32_t i = buf_start; i < buf_stop; ++i)
    {
        buf[0][cur_cstate_buf_pos] = in_data[0][i];
        buf[1][cur_cstate_buf_pos] = in_data[1][i];

        ++cur_cstate_buf_pos;
        if (cur_cstate_buf_pos >= delay_buf_size)
        {
            assert(cur_cstate_buf_pos == delay_buf_size);
            cur_cstate_buf_pos = 0;
        }
    }

    cstate->buf_pos = cur_cstate_buf_pos;

    return;
}


static void del_Proc_chorus(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_chorus* chorus = (Proc_chorus*)dimpl;
    memory_free(chorus);

    return;
}


