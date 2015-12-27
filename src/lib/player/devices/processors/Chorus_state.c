

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Chorus_state.h>

#include <Audio_buffer.h>
#include <devices/processors/Proc_chorus.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/Linear_controls.h>
#include <player/Player.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Chorus_voice
{
    Linear_controls delay_variance;
    Linear_controls volume_controls;
    double delay;
    double range;
    double speed;
    double volume;
} Chorus_voice;


typedef struct Chorus_pstate
{
    Proc_state parent;

    Audio_buffer* buf;
    int32_t buf_pos;
    Chorus_voice voices[CHORUS_VOICES_MAX];
} Chorus_pstate;


static void Chorus_voice_reset(
        Chorus_voice* voice, const Chorus_voice_params* params, int32_t audio_rate)
{
    assert(voice != NULL);
    assert(params != NULL);
    ignore(audio_rate);

    Linear_controls_init(&voice->delay_variance);
    Linear_controls_set_audio_rate(&voice->delay_variance, audio_rate);
    Linear_controls_set_tempo(&voice->delay_variance, DEFAULT_TEMPO);
    voice->delay = params->delay;
    voice->range = params->range;
    voice->speed = params->speed;
    voice->volume = params->volume;

    Linear_controls_init(&voice->volume_controls);
    Linear_controls_set_audio_rate(&voice->volume_controls, audio_rate);
    Linear_controls_set_tempo(&voice->volume_controls, DEFAULT_TEMPO);
    Linear_controls_set_range(&voice->volume_controls, -INFINITY, CHORUS_DB_MAX);
    Linear_controls_set_value(&voice->volume_controls, voice->volume);

    if ((voice->delay < 0) || (voice->delay >= CHORUS_DELAY_MAX))
        return;

    Linear_controls_set_value(&voice->delay_variance, voice->delay);

    Linear_controls_osc_depth_value(
            &voice->delay_variance, max(voice->range, -voice->delay));

    Linear_controls_osc_speed_value(&voice->delay_variance, voice->speed);

    return;
}


static void Chorus_pstate_deinit(Device_state* dev_state)
{
    assert(dev_state != NULL);

    Chorus_pstate* cstate = (Chorus_pstate*)dev_state;
    if (cstate->buf != NULL)
    {
        del_Audio_buffer(cstate->buf);
        cstate->buf = NULL;
    }

    Proc_state_deinit(&cstate->parent.parent);

    return;
}


static bool Chorus_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;

    const int32_t delay_buf_size = CHORUS_BUF_TIME * audio_rate + 1;

    assert(cstate->buf != NULL);
    if (!Audio_buffer_resize(cstate->buf, delay_buf_size))
        return false;

    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        Chorus_voice* voice = &cstate->voices[i];
        Linear_controls_set_audio_rate(&voice->delay_variance, audio_rate);
        Linear_controls_set_audio_rate(&voice->volume_controls, audio_rate);
    }

    return true;
}


static void Chorus_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    const Proc_chorus* chorus = (Proc_chorus*)dstate->device->dimpl;

    const uint32_t delay_buf_size = Audio_buffer_get_size(cstate->buf);
    Audio_buffer_clear(cstate->buf, 0, delay_buf_size);
    cstate->buf_pos = 0;

    for (int i = 0; i < CHORUS_VOICES_MAX; ++i)
    {
        const Chorus_voice_params* params = &chorus->voice_params[i];
        Chorus_voice* voice = &cstate->voices[i];
        Chorus_voice_reset(voice, params, cstate->parent.parent.audio_rate);
    }

    return;
}


static void Chorus_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start <= buf_stop);
    assert(tempo > 0);

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;

    const float* in_data[] = { NULL, NULL };
    float* out_data[] = { NULL, NULL };
    get_raw_input(&cstate->parent.parent, 0, in_data);
    get_raw_output(&cstate->parent.parent, 0, out_data);

    float* buf[] =
    {
        Audio_buffer_get_buffer(cstate->buf, 0),
        Audio_buffer_get_buffer(cstate->buf, 1),
    };

    const int32_t delay_buf_size = Audio_buffer_get_size(cstate->buf);
    const int32_t delay_max = delay_buf_size - 1;

    static const int CHORUS_WORK_BUFFER_TOTAL_OFFSETS = WORK_BUFFER_IMPL_1;
    static const int CHORUS_WORK_BUFFER_DELAY = WORK_BUFFER_IMPL_2;
    static const int CHORUS_WORK_BUFFER_VOLUME = WORK_BUFFER_IMPL_3;

    float* total_offsets = Work_buffers_get_buffer_contents_mut(
            wbs, CHORUS_WORK_BUFFER_TOTAL_OFFSETS);

    const Work_buffer* delays_wb =
        Work_buffers_get_buffer(wbs, CHORUS_WORK_BUFFER_DELAY);
    float* delays = Work_buffer_get_contents_mut(delays_wb);

    const Work_buffer* vols_wb = Work_buffers_get_buffer(wbs, CHORUS_WORK_BUFFER_VOLUME);
    float* vols = Work_buffer_get_contents_mut(vols_wb);

    int32_t cur_cstate_buf_pos = cstate->buf_pos;

    const int32_t audio_rate = dstate->audio_rate;

    // Mix chorus voices
    for (int vi = 0; vi < CHORUS_VOICES_MAX; ++vi)
    {
        Chorus_voice* voice = &cstate->voices[vi];
        if ((voice->delay < 0) || (voice->delay >= CHORUS_DELAY_MAX))
            continue;

        Linear_controls_set_tempo(&voice->delay_variance, tempo);
        Linear_controls_set_tempo(&voice->volume_controls, tempo);

        Linear_controls_fill_work_buffer(
                &voice->delay_variance, delays_wb, buf_start, buf_stop);

        Linear_controls_fill_work_buffer(
                &voice->volume_controls, vols_wb, buf_start, buf_stop);

        // Get total offsets
        for (int32_t i = buf_start, chunk_offset = 0; i < buf_stop; ++i, ++chunk_offset)
        {
            const double delay = delays[i];
            double delay_frames = delay * CHORUS_DELAY_SCALE * audio_rate;
            delay_frames = clamp(delay_frames, 0, delay_max);
            total_offsets[i] = chunk_offset - delay_frames;
        }

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            const float total_offset = total_offsets[i];
            const float volume = dB_to_scale(vols[i]);

            // Get buffer positions
            const int32_t cur_pos = (int32_t)floor(total_offset);
            const double remainder = total_offset - cur_pos;
            assert(cur_pos <= (int32_t)i);
            assert(implies(cur_pos == (int32_t)i, remainder == 0));
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
            const float val_l =
                (prev_scale * volume * cur_val_l) + (remainder * volume * next_val_l);
            const float val_r =
                (prev_scale * volume * cur_val_r) + (remainder * volume * next_val_r);

            out_data[0][i] += val_l;
            out_data[1][i] += val_r;
        }
    }

    // Update the chorus state buffers
    for (int32_t i = buf_start; i < buf_stop; ++i)
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


static void Chorus_pstate_clear_history(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Chorus_pstate* cstate = (Chorus_pstate*)proc_state;
    Audio_buffer_clear(cstate->buf, 0, Audio_buffer_get_size(cstate->buf));

    cstate->buf_pos = 0;

    return;
}


Device_state* new_Chorus_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Chorus_pstate* cstate = memory_alloc_item(Chorus_pstate);
    if (cstate == NULL)
        return NULL;

    if (!Proc_state_init(&cstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(cstate);
        return NULL;
    }

    cstate->parent.parent.deinit = Chorus_pstate_deinit;
    cstate->parent.set_audio_rate = Chorus_pstate_set_audio_rate;
    cstate->parent.reset = Chorus_pstate_reset;
    cstate->parent.render_mixed = Chorus_pstate_render_mixed;
    cstate->parent.clear_history = Chorus_pstate_clear_history;
    cstate->buf = NULL;
    cstate->buf_pos = 0;

    const int32_t delay_buf_size = CHORUS_BUF_TIME * audio_rate + 1;

    cstate->buf = new_Audio_buffer(delay_buf_size);
    if (cstate->buf == NULL)
    {
        del_Device_state(&cstate->parent.parent);
        return NULL;
    }

    return &cstate->parent.parent;
}


Linear_controls* Chorus_pstate_get_cv_delay_variance(
        Device_state* dstate, const Key_indices indices)
{
    assert(dstate != NULL);
    assert(indices != NULL);

    const int index = indices[0];
    if ((index < 0) || (index >= CHORUS_VOICES_MAX))
        return NULL;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[index];

    return &voice->delay_variance;
}


Linear_controls* Chorus_pstate_get_cv_volume(
        Device_state* dstate, const Key_indices indices)
{
    assert(dstate != NULL);
    assert(indices != NULL);

    const int index = indices[0];
    if ((index < 0) || (index >= CHORUS_VOICES_MAX))
        return NULL;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[index];

    return &voice->volume_controls;
}


static const Chorus_voice_params* get_voice_params(
        const Device_state* dstate, const Key_indices indices)
{
    assert(dstate != NULL);
    assert(indices != NULL);

    const int32_t index = indices[0];
    assert(index >= 0);
    assert(index < CHORUS_VOICES_MAX);

    const Proc_chorus* chorus = (Proc_chorus*)dstate->device->dimpl;
    const Chorus_voice_params* params = &chorus->voice_params[index];

    return params;
}


bool Chorus_pstate_set_voice_delay(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    ignore(value);

    Linear_controls* controls = Chorus_pstate_get_cv_delay_variance(dstate, indices);
    if (controls == NULL)
        return true;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    voice->delay = get_voice_params(dstate, indices)->delay;

    if (voice->delay < 0.0)
    {
        Linear_controls_init(controls);
    }
    else
    {
        Linear_controls_set_value(controls, voice->delay);
        Linear_controls_osc_speed_value(controls, voice->speed);
        Linear_controls_osc_depth_value(controls, max(-voice->delay, voice->range));
    }

    return true;
}


bool Chorus_pstate_set_voice_range(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    ignore(value);

    Linear_controls* controls = Chorus_pstate_get_cv_delay_variance(dstate, indices);
    if (controls == NULL)
        return true;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    voice->range = get_voice_params(dstate, indices)->range;

    if (voice->delay >= 0.0)
    {
        Linear_controls_set_value(controls, voice->delay);
        Linear_controls_osc_speed_value(controls, voice->speed);
        Linear_controls_osc_depth_value(controls, max(-voice->delay, voice->range));
    }

    return true;
}


bool Chorus_pstate_set_voice_speed(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    ignore(value);

    Linear_controls* controls = Chorus_pstate_get_cv_delay_variance(dstate, indices);
    if (controls == NULL)
        return true;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    voice->speed = get_voice_params(dstate, indices)->speed;

    Linear_controls_osc_speed_value(controls, voice->speed);

    return true;
}


bool Chorus_pstate_set_voice_volume(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);
    ignore(value);

    Linear_controls* controls = Chorus_pstate_get_cv_volume(dstate, indices);
    if (controls == NULL)
        return true;

    Chorus_pstate* cstate = (Chorus_pstate*)dstate;
    Chorus_voice* voice = &cstate->voices[indices[0]];
    voice->volume = get_voice_params(dstate, indices)->volume;

    Linear_controls_set_value(controls, voice->volume);

    return true;
}


