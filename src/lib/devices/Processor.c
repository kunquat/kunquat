

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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <debug/assert.h>
#include <devices/Proc_common.h>
#include <devices/Proc_type.h>
#include <devices/Processor.h>
#include <Filter.h>
#include <memory.h>
#include <pitch_t.h>


static Device_state* Processor_create_state_plain(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* proc_state = memory_alloc_item(Proc_state);
    if (proc_state == NULL)
        return NULL;

    if (!Proc_state_init(proc_state, device, audio_rate, audio_buffer_size))
    {
        memory_free(proc_state);
        return NULL;
    }

    return &proc_state->parent;
}


Processor* new_Processor(const Au_params* au_params)
{
    assert(au_params != NULL);

    Processor* proc = memory_alloc_item(Processor);
    if (proc == NULL)
        return NULL;

    if (!Device_init(&proc->parent, true))
    {
        memory_free(proc);
        return NULL;
    }

    //fprintf(stderr, "New Processor %p\n", (void*)proc);
    proc->au_params = au_params;
    proc->voice_features = VOICE_FEATURES_ALL;

    proc->init_vstate = NULL;
    proc->process_vstate = NULL;
    proc->clear_history = NULL;

    Device_set_state_creator(
            &proc->parent,
            Processor_create_state_plain);

    return proc;
}


bool Processor_init(
        Processor* proc,
        Proc_process_vstate_func process_vstate,
        void (*init_vstate)(const Processor*, const Proc_state*, Voice_state*),
        Device_process_signal_func* process_signal)
{
    assert(proc != NULL);

    proc->process_vstate = process_vstate;
    proc->init_vstate = init_vstate;
    Device_set_process(&proc->parent, process_signal);

    return true;
}


void Processor_set_voice_feature(Processor* proc, Voice_feature feature, bool enabled)
{
    assert(proc != NULL);
    assert(feature < VOICE_FEATURE_COUNT_);

    if (enabled)
        proc->voice_features |= VOICE_FEATURE_FLAG(feature);
    else
        proc->voice_features &= ~VOICE_FEATURE_FLAG(feature);

    return;
}


bool Processor_is_voice_feature_enabled(const Processor* proc, Voice_feature feature)
{
    assert(proc != NULL);
    assert(feature < VOICE_FEATURE_COUNT_);

    return (proc->voice_features & VOICE_FEATURE_FLAG(feature)) != 0;
}


void Processor_set_clear_history(
        Processor* proc, void (*func)(const Device_impl*, Proc_state*))
{
    assert(proc != NULL);
    assert(func != NULL);

    proc->clear_history = func;

    return;
}


void Processor_clear_history(const Processor* proc, Proc_state* proc_state)
{
    assert(proc != NULL);

    if ((proc->clear_history != NULL) && (proc->parent.dimpl != NULL))
        proc->clear_history(proc->parent.dimpl, proc_state);

    return;
}


/**
 * Update voice parameter settings that depend on audio rate and/or tempo.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param audio_rate   The new audio rate -- must be positive.
 * \param tempo        The new tempo -- must be positive and finite.
 */
static void adjust_relative_lengths(
        Voice_state* vstate, uint32_t audio_rate, double tempo)
{
    assert(vstate != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    if (vstate->freq != audio_rate || vstate->tempo != tempo)
    {
        Slider_set_mix_rate(&vstate->pitch_slider, audio_rate);
        Slider_set_tempo(&vstate->pitch_slider, tempo);
        LFO_set_mix_rate(&vstate->vibrato, audio_rate);
        LFO_set_tempo(&vstate->vibrato, tempo);

        if (vstate->arpeggio)
        {
            vstate->arpeggio_length *= (double)audio_rate / vstate->freq;
            vstate->arpeggio_length *= vstate->tempo / tempo;
            vstate->arpeggio_frames *= (double)audio_rate / vstate->freq;
            vstate->arpeggio_frames *= vstate->tempo / tempo;
        }

        Slider_set_mix_rate(&vstate->force_slider, audio_rate);
        Slider_set_tempo(&vstate->force_slider, tempo);
        LFO_set_mix_rate(&vstate->tremolo, audio_rate);
        LFO_set_tempo(&vstate->tremolo, tempo);

        Slider_set_mix_rate(&vstate->panning_slider, audio_rate);
        Slider_set_tempo(&vstate->panning_slider, tempo);

        Slider_set_mix_rate(&vstate->lowpass_slider, audio_rate);
        Slider_set_tempo(&vstate->lowpass_slider, tempo);
        LFO_set_mix_rate(&vstate->autowah, audio_rate);
        LFO_set_tempo(&vstate->autowah, tempo);

        vstate->freq = audio_rate;
        vstate->tempo = tempo;
    }

    return;
}


bool Processor_get_vstate_support(const Processor* proc)
{
    assert(proc != NULL);
    return (proc->process_vstate != NULL);
}


const Au_params* Processor_get_au_params(const Processor* proc)
{
    assert(proc != NULL);
    return proc->au_params;
}


void Processor_process_vstate(
        const Processor* proc,
        Device_states* dstates,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    assert(proc->process_vstate != NULL);
    assert(dstates != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    // Get processor state
    Proc_state* proc_state = (Proc_state*)Device_states_get_state(
            dstates, Device_get_id(&proc->parent));

    // Get voice output buffer and make sure it is cleared
    Audio_buffer* voice_out_buf = Proc_state_get_output_voice_buffer(proc_state);
    Audio_buffer_clear(voice_out_buf, buf_start, buf_stop);
    kqt_frame* voice_out_l = Audio_buffer_get_buffer(voice_out_buf, 0);
    kqt_frame* voice_out_r = Audio_buffer_get_buffer(voice_out_buf, 1);

    if (!vstate->active)
        return;

    // Check for voice cut before mixing anything (no need for volume ramping)
    if (!vstate->note_on &&
            (vstate->pos == 0) &&
            (vstate->pos_rem == 0) &&
            !proc->au_params->env_force_rel_enabled)
    {
        vstate->active = false;
        return;
    }

    if (buf_start >= buf_stop)
        return;

    // Get audio unit state
    Au_state* au_state = (Au_state*)Device_states_get_state(
            dstates, proc->au_params->device_id);

    // Get audio output buffers
    Audio_buffer* audio_buffer = Device_state_get_audio_buffer(
            &proc_state->parent, DEVICE_PORT_TYPE_SEND, 0);
    if (audio_buffer == NULL)
    {
        vstate->active = false;
        return;
    }

    // Process common parameters required by implementations
    bool deactivate_after_processing = false;
    int32_t process_stop = buf_stop;

    adjust_relative_lengths(vstate, audio_rate, tempo);

    Proc_common_handle_pitch(proc, vstate, wbs, buf_start, process_stop);

    const int32_t force_stop = Proc_common_handle_force(
            proc, au_state, vstate, wbs, audio_rate, buf_start, process_stop);

    const bool force_ended = (force_stop < process_stop);
    if (force_ended)
    {
        deactivate_after_processing = true;
        assert(force_stop <= process_stop);
        process_stop = force_stop;
    }

    const uint64_t old_pos = vstate->pos;
    const double old_pos_rem = vstate->pos_rem;

    // Call the implementation
    const int32_t impl_render_stop = proc->process_vstate(
            proc,
            proc_state,
            au_state,
            vstate,
            wbs,
            buf_start,
            process_stop,
            audio_rate,
            tempo);
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
    const int32_t ramp_release_stop = Proc_common_ramp_release(
            proc, au_state, vstate, wbs, 2, audio_rate, buf_start, process_stop);
    const bool ramp_release_ended = (vstate->ramp_release >= 1);
    if (ramp_release_ended)
    {
        deactivate_after_processing = true;
        assert(ramp_release_stop <= process_stop);
        process_stop = ramp_release_stop;
    }

    Proc_common_handle_filter(
            proc, vstate, wbs, 2, audio_rate, buf_start, process_stop);
    Proc_common_handle_panning(proc, vstate, wbs, buf_start, process_stop);

    vstate->pos = new_pos;
    vstate->pos_rem = new_pos_rem;

    // Copy and mix rendered audio to output buffers
    {
        const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_L);
        const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_R);
        float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
        float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

        for (int32_t i = buf_start; i < process_stop; ++i)
            voice_out_l[i] = audio_l[i];
        for (int32_t i = buf_start; i < process_stop; ++i)
            voice_out_r[i] = audio_r[i];

        Audio_buffer_mix(audio_buffer, voice_out_buf, buf_start, buf_stop);

        /*
        fprintf(stderr, "1st item by %d @ %p: %.1f\n",
                (int)Device_get_id((const Device*)proc),
                (const void*)audio_buffer,
                out_l[buf_start]);
        // */
    }

    if (deactivate_after_processing)
        vstate->active = false;

    return;
}


void del_Processor(Processor* proc)
{
    if (proc == NULL)
        return;

    Device_deinit(&proc->parent);
    memory_free(proc);

    return;
}


