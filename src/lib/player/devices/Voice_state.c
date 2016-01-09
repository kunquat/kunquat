

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Voice_state.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <player/devices/Voice_state_common.h>
#include <player/Slider.h>

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


Voice_state* Voice_state_init(
        Voice_state* state,
        Random* rand_p,
        Random* rand_s,
        int32_t audio_rate,
        double tempo)
{
    assert(state != NULL);
    assert(rand_p != NULL);
    assert(rand_s != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    Voice_state_clear(state);
    state->active = true;
    state->has_finished = false;
    state->note_on = true;
    state->freq = audio_rate;
    state->tempo = tempo;
    state->rand_p = rand_p;
    state->rand_s = rand_s;

    state->render_voice = NULL;

    state->has_release_data = false;
    state->release_stop = 0;

    Pitch_controls_init(&state->pitch_controls, audio_rate, tempo);

    state->is_force_state = false;

    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);

    state->active = false;
    state->has_finished = false;
    state->freq = 0;
    state->tempo = 0;
    state->ramp_attack = 0;

    state->hit_index = -1;
    Pitch_controls_reset(&state->pitch_controls);
    state->orig_pitch_param = NAN;
    state->actual_pitch = 0;
    state->prev_actual_pitch = 0;

    state->arpeggio = false;
    state->arpeggio_ref = NAN;
    state->arpeggio_length = 0;
    state->arpeggio_frames = 0;
    state->arpeggio_note = 0;
    state->arpeggio_tones[0] = state->arpeggio_tones[1] = NAN;
#if 0
    for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i)
    {
        state->arpeggio_offsets[i] = NAN;
    }
#endif

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;

    state->is_force_state = false;

    return state;
}


/**
 * Update voice parameter settings that depend on audio rate and/or tempo.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param audio_rate   The new audio rate -- must be positive.
 * \param tempo        The new tempo -- must be positive and finite.
 */
static void adjust_relative_lengths(
        Voice_state* vstate, int32_t audio_rate, double tempo)
{
    assert(vstate != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    if (vstate->freq != audio_rate || vstate->tempo != tempo)
    {
        Pitch_controls_set_audio_rate(&vstate->pitch_controls, audio_rate);
        Pitch_controls_set_tempo(&vstate->pitch_controls, tempo);

        if (vstate->arpeggio)
        {
            vstate->arpeggio_length *= (double)audio_rate / vstate->freq;
            vstate->arpeggio_length *= vstate->tempo / tempo;
            vstate->arpeggio_frames *= (double)audio_rate / vstate->freq;
            vstate->arpeggio_frames *= vstate->tempo / tempo;
        }

        vstate->freq = audio_rate;
        vstate->tempo = tempo;
    }

    return;
}


int32_t Voice_state_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    vstate->has_release_data = false;
    vstate->release_stop = buf_start;

    const Processor* proc = (const Processor*)proc_state->parent.device;
    if (!Processor_get_voice_signals(proc) || (vstate->render_voice == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    if (buf_start >= buf_stop)
        return buf_start;

    // Get audio output buffers
    Work_buffer* audio_buffer = Device_state_get_audio_buffer(
            &proc_state->parent, DEVICE_PORT_TYPE_SEND, 0);
    if (audio_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    Work_buffer* voice_out_buf = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (voice_out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Process common parameters required by implementations
    int32_t process_stop = buf_stop;

    const int32_t audio_rate = proc_state->parent.audio_rate;

    adjust_relative_lengths(vstate, audio_rate, tempo);

    // NOTE: checking voice features before processing in order to save time
    //       Revisit if we ever add support for several output ports!
    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH))
        Voice_state_common_handle_pitch(vstate, proc, wbs, buf_start, process_stop);

    // Call the implementation
    const int32_t impl_render_stop = vstate->render_voice(
            vstate, proc_state, au_state, wbs, buf_start, process_stop, tempo);

    if (!vstate->active)
    {
        assert(impl_render_stop <= process_stop);
        process_stop = impl_render_stop;
    }

    return process_stop;
}


void Voice_state_mix_signals(
        Voice_state* vstate,
        Proc_state* proc_state,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= buf_start);

    for (int32_t port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* mixed_buffer = Device_state_get_audio_buffer(
                &proc_state->parent, DEVICE_PORT_TYPE_SEND, port);
        const Work_buffer* voice_buffer = Proc_state_get_voice_buffer(
                proc_state, DEVICE_PORT_TYPE_SEND, port);

        if ((mixed_buffer != NULL) && (voice_buffer != NULL))
            Work_buffer_mix(mixed_buffer, voice_buffer, buf_start, buf_stop);
    }

    return;
}


void Voice_state_mark_release_data(Voice_state* vstate, int32_t release_stop)
{
    assert(vstate != NULL);
    assert(release_stop >= 0);

    vstate->has_release_data = true;
    vstate->release_stop = release_stop;

    return;
}


void Voice_state_set_finished(Voice_state* vstate)
{
    assert(vstate != NULL);
    vstate->has_finished = true;
    return;
}


void Voice_state_cv_generic_set(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Value* value)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(value != NULL);

    const Device_impl* dimpl = dstate->device->dimpl;

    Device_impl_voice_cv_callback* cb = DEVICE_IMPL_VOICE_CV_CALLBACK_AUTO;
    Device_impl_get_voice_cv_callback(dimpl, key, value->type, cb);

    if (cb->type == VALUE_TYPE_NONE)
        return;

    switch (cb->type)
    {
        case VALUE_TYPE_BOOL:
        {
            cb->cb.set_bool(vstate, dstate, cb->indices, value->value.bool_type);
        }
        break;

        case VALUE_TYPE_INT:
        {
            cb->cb.set_int(vstate, dstate, cb->indices, value->value.int_type);
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            Linear_controls* controls =
                cb->cb.get_float_controls(vstate, dstate, cb->indices);
            if (controls != NULL)
                Linear_controls_set_value(controls, value->value.float_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            cb->cb.set_tstamp(vstate, dstate, cb->indices, &value->value.Tstamp_type);
        }
        break;

        default:
            assert(false);
    }

    return;
}


static Linear_controls* get_cv_float_controls(
        Voice_state* vstate, const Device_state* dstate, const char* key)
{
    assert(dstate != NULL);
    assert(key != NULL);

    const Device_impl* dimpl = dstate->device->dimpl;

    Device_impl_voice_cv_callback* cb = DEVICE_IMPL_VOICE_CV_CALLBACK_AUTO;
    Device_impl_get_voice_cv_callback(dimpl, key, VALUE_TYPE_FLOAT, cb);

    if (cb->type != VALUE_TYPE_FLOAT)
        return NULL;

    return cb->cb.get_float_controls(vstate, dstate, cb->indices);
}


void Voice_state_cv_float_init(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Linear_controls* src_controls)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(src_controls != NULL);

    Linear_controls* dest_controls = get_cv_float_controls(vstate, dstate, key);
    if (dest_controls != NULL)
        Linear_controls_copy(dest_controls, src_controls);

    return;
}


void Voice_state_cv_float_slide_target(
        Voice_state* vstate, const Device_state* dstate, const char* key, double value)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(value));

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_slide_value_target(controls, value);

    return;
}


void Voice_state_cv_float_slide_length(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_slide_value_length(controls, length);

    return;
}


void Voice_state_cv_float_osc_speed(
        Voice_state* vstate, const Device_state* dstate, const char* key, double speed)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(speed));
    assert(speed >= 0);

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_osc_speed_value(controls, speed);

    return;
}


void Voice_state_cv_float_osc_depth(
        Voice_state* vstate, const Device_state* dstate, const char* key, double depth)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(depth));

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_osc_depth_value(controls, depth);

    return;
}


void Voice_state_cv_float_osc_speed_slide(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_osc_speed_slide_value(controls, length);

    return;
}


void Voice_state_cv_float_osc_depth_slide(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Tstamp* length)
{
    assert(vstate != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(vstate, dstate, key);
    if (controls != NULL)
        Linear_controls_osc_depth_slide_value(controls, length);

    return;
}


