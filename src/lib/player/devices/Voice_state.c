

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
        int32_t freq,
        double tempo)
{
    assert(state != NULL);
    assert(rand_p != NULL);
    assert(rand_s != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Voice_state_clear(state);
    state->active = true;
    state->note_on = true;
    state->freq = freq;
    state->tempo = tempo;
    state->rand_p = rand_p;
    state->rand_s = rand_s;

    state->render_voice = NULL;

    Force_controls_init(&state->force_controls, freq, tempo);
    Pitch_controls_init(&state->pitch_controls, freq, tempo);
    Filter_controls_init(&state->filter_controls, freq, tempo);

    Slider_set_audio_rate(&state->panning_slider, freq);
    Slider_set_tempo(&state->panning_slider, tempo);

    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);

    state->active = false;
    state->freq = 0;
    state->tempo = 0;
    state->ramp_attack = 0;
    state->ramp_release = 0;

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

    Time_env_state_init(&state->force_env_state);
    Time_env_state_init(&state->force_rel_env_state);

    Force_controls_reset(&state->force_controls);
    state->actual_force = 1;

    state->panning = 0;
    state->actual_panning = 0;
    Slider_init(&state->panning_slider, SLIDE_MODE_LINEAR);

    state->pitch_pan_ref_param = FLT_MAX;
    state->pitch_pan_value = 0;

    Time_env_state_init(&state->env_filter_state);
    Time_env_state_init(&state->env_filter_rel_state);

    Filter_controls_reset(&state->filter_controls);
    state->actual_lowpass = 100;

    state->applied_lowpass = state->actual_lowpass;
    state->applied_resonance = state->filter_controls.resonance;
    state->true_lowpass = INFINITY;
    state->true_resonance = 0.5;
    state->lowpass_state_used = -1;
    state->lowpass_xfade_state_used = -1;
    state->lowpass_xfade_pos = 1;
    state->lowpass_xfade_update = 0;

    for (int i = 0; i < FILTER_ORDER; ++i)
    {
        state->lowpass_state[0].coeffs[i] = 0;
        state->lowpass_state[1].coeffs[i] = 0;
        for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
        {
            state->lowpass_state[0].history1[k][i] = 0;
            state->lowpass_state[0].history2[k][i] = 0;
            state->lowpass_state[1].history1[k][i] = 0;
            state->lowpass_state[1].history2[k][i] = 0;
        }
    }

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

        Force_controls_set_audio_rate(&vstate->force_controls, audio_rate);
        Force_controls_set_tempo(&vstate->force_controls, tempo);

        Slider_set_audio_rate(&vstate->panning_slider, audio_rate);
        Slider_set_tempo(&vstate->panning_slider, tempo);

        Filter_controls_set_audio_rate(&vstate->filter_controls, audio_rate);
        Filter_controls_set_tempo(&vstate->filter_controls, tempo);

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

    const Processor* proc = (const Processor*)proc_state->parent.device;
    if (!Processor_get_voice_signals(proc) || (vstate->render_voice == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    // Check for voice cut before mixing anything (no need for volume ramping)
    if (!vstate->note_on &&
            (vstate->pos == 0) &&
            (vstate->pos_rem == 0) &&
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_CUT) &&
            (!proc->au_params->env_force_rel_enabled ||
                !Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE)))
    {
        vstate->active = false;
        return buf_start;
    }

    if (buf_start >= buf_stop)
        return buf_start;

    // Get audio output buffers
    Audio_buffer* audio_buffer = Device_state_get_audio_buffer(
            &proc_state->parent, DEVICE_PORT_TYPE_SEND, 0);
    if (audio_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    Audio_buffer* voice_out_buf = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (voice_out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    // Process common parameters required by implementations
    bool deactivate_after_processing = false;
    int32_t process_stop = buf_stop;

    const int32_t audio_rate = proc_state->parent.audio_rate;

    adjust_relative_lengths(vstate, audio_rate, tempo);

    // NOTE: checking voice features before processing in order to save time
    //       Revisit if we ever add support for several output ports!
    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH))
        Voice_state_common_handle_pitch(vstate, proc, wbs, buf_start, process_stop);

    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
    {
        const int32_t force_stop = Voice_state_common_handle_force(
                vstate, au_state, proc, wbs, audio_rate, buf_start, process_stop);

        const bool force_ended = (force_stop < process_stop);
        if (force_ended)
        {
            deactivate_after_processing = true;
            assert(force_stop <= process_stop);
            process_stop = force_stop;
        }
    }

    const uint64_t old_pos = vstate->pos;
    const double old_pos_rem = vstate->pos_rem;

    // Call the implementation
    const int32_t impl_render_stop = vstate->render_voice(
            vstate, proc_state, au_state, wbs, buf_start, process_stop, tempo);

    if (!vstate->active) // FIXME: communicate end of rendering in a cleaner way
    {
        vstate->active = true;
        deactivate_after_processing = true;
        assert(impl_render_stop <= process_stop);
        process_stop = impl_render_stop;
    }

    // XXX: Hack to make post-processing work correctly below, fix properly!
    const uint64_t new_pos = vstate->pos;
    const double new_pos_rem = vstate->pos_rem;
    vstate->pos = old_pos;
    vstate->pos_rem = old_pos_rem;

    // Apply common parameters to generated signal
    // TODO: does not work with multiple voice output ports
    if (Proc_state_is_voice_out_buffer_modified(proc_state, 0))
    {
        const int32_t ramp_release_stop = Voice_state_common_ramp_release(
                vstate,
                voice_out_buf,
                proc,
                au_state,
                wbs,
                2,
                audio_rate,
                buf_start,
                process_stop);
        const bool ramp_release_ended = (vstate->ramp_release >= 1);
        if (ramp_release_ended)
        {
            deactivate_after_processing = true;
            assert(ramp_release_stop <= process_stop);
            process_stop = ramp_release_stop;
        }

        Voice_state_common_handle_filter(
                vstate,
                voice_out_buf,
                proc,
                au_state,
                wbs,
                2,
                audio_rate,
                buf_start,
                process_stop);

        Voice_state_common_handle_panning(
                vstate, voice_out_buf, proc, wbs, buf_start, process_stop);
    }

    vstate->pos = new_pos;
    vstate->pos_rem = new_pos_rem;

    // Mix rendered audio to the combined signal buffer
    {
        Audio_buffer_mix(audio_buffer, voice_out_buf, buf_start, process_stop);

        /*
        fprintf(stderr, "1st item by %d @ %p: %.1f\n",
                (int)Device_get_id((const Device*)proc),
                (const void*)audio_buffer,
                out_l[buf_start]);
        // */
    }

    if (deactivate_after_processing)
        vstate->active = false;

    return process_stop;
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


