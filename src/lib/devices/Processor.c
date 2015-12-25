

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


#include <devices/Processor.h>

#include <debug/assert.h>
#include <devices/Proc_common.h>
#include <devices/Proc_type.h>
#include <Filter.h>
#include <memory.h>
#include <player/Channel.h>
#include <player/devices/Voice_state.h>
#include <player/Voice.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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


static void Processor_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* key,
        const Value* value);

static void Processor_slide_control_var_float_target(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double value);

static void Processor_slide_control_var_float_length(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length);

static void Processor_osc_speed_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double speed);

static void Processor_osc_depth_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double depth);

static void Processor_osc_speed_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length);

static void Processor_osc_depth_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length);

static void Processor_init_control_var_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Linear_controls* src_controls);


Processor* new_Processor(int index, const Au_params* au_params)
{
    assert(index >= 0);
    assert(index < KQT_PROCESSORS_MAX);
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
    proc->index = index;
    proc->au_params = au_params;
    for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
        proc->voice_features[port_num] = VOICE_FEATURES_ALL;

    proc->enable_voice_support = false;
    proc->enable_signal_support = false;

    proc->init_vstate = NULL;

    Device_set_state_creator(
            &proc->parent,
            Processor_create_state_plain);

    Device_register_set_control_var_generic(
            &proc->parent, Processor_set_control_var_generic);

    Device_register_slide_control_var_float(
            &proc->parent,
            Processor_slide_control_var_float_target,
            Processor_slide_control_var_float_length);

    Device_register_osc_cv_float(
            &proc->parent,
            Processor_osc_speed_cv_float,
            Processor_osc_depth_cv_float,
            Processor_osc_speed_slide_cv_float,
            Processor_osc_depth_slide_cv_float);

    Device_register_init_control_var_float(
            &proc->parent, Processor_init_control_var_float);

    return proc;
}


bool Processor_init(Processor* proc, Voice_state_init_func* init_vstate)
{
    assert(proc != NULL);

    proc->init_vstate = init_vstate;

    return true;
}


void Processor_set_voice_feature(
        Processor* proc, int port_num, Voice_feature feature, bool enabled)
{
    assert(proc != NULL);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);
    assert(feature < VOICE_FEATURE_COUNT_);

    if (enabled)
        proc->voice_features[port_num] |= VOICE_FEATURE_FLAG(feature);
    else
        proc->voice_features[port_num] &= ~VOICE_FEATURE_FLAG(feature);

    return;
}


bool Processor_is_voice_feature_enabled(
        const Processor* proc, int port_num, Voice_feature feature)
{
    assert(proc != NULL);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);
    assert(feature < VOICE_FEATURE_COUNT_);

    return (proc->voice_features[port_num] & VOICE_FEATURE_FLAG(feature)) != 0;
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


void Processor_set_voice_signals(Processor* proc, bool enabled)
{
    assert(proc != NULL);
    proc->enable_voice_support = enabled;
    return;
}


bool Processor_get_voice_signals(const Processor* proc)
{
    assert(proc != NULL);
    return proc->enable_voice_support;
}


void Processor_set_signal_support(Processor* proc, bool enabled)
{
    assert(proc != NULL);
    proc->enable_signal_support = enabled;
    return;
}


bool Processor_get_signal_support(const Processor* proc)
{
    assert(proc != NULL);
    return proc->enable_signal_support;
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
    assert(dstates != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    // Get processor state
    Proc_state* proc_state = (Proc_state*)Device_states_get_state(
            dstates, Device_get_id(&proc->parent));

    if (!vstate->active)
        return;

    // Check for voice cut before mixing anything (no need for volume ramping)
    if (!vstate->note_on &&
            (vstate->pos == 0) &&
            (vstate->pos_rem == 0) &&
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_CUT) &&
            (!proc->au_params->env_force_rel_enabled ||
                !Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE)))
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

    Audio_buffer* voice_out_buf = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (voice_out_buf == NULL)
    {
        vstate->active = false;
        return;
    }

    // Process common parameters required by implementations
    bool deactivate_after_processing = false;
    int32_t process_stop = buf_stop;

    adjust_relative_lengths(vstate, audio_rate, tempo);

    // NOTE: checking voice features before processing in order to save time
    //       Revisit if we ever add support for several output ports!
    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH))
        Proc_common_handle_pitch(proc, vstate, wbs, buf_start, process_stop);

    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
    {
        const int32_t force_stop = Proc_common_handle_force(
                proc, au_state, vstate, wbs, audio_rate, buf_start, process_stop);

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
    const int32_t impl_render_stop = Voice_state_render_voice(
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
        const int32_t ramp_release_stop = Proc_common_ramp_release(
                proc,
                au_state,
                vstate,
                wbs,
                voice_out_buf,
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

        Proc_common_handle_filter(
                proc,
                au_state,
                vstate,
                wbs,
                voice_out_buf,
                2,
                audio_rate,
                buf_start,
                process_stop);

        Proc_common_handle_panning(
                proc, vstate, wbs, voice_out_buf, buf_start, process_stop);
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

    return;
}


static void Processor_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* key,
        const Value* value)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(key != NULL);
    assert(value != NULL);

    const Processor* proc = (const Processor*)device;
    const Device_impl* dimpl = device->dimpl;
    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        if (value->type == VALUE_TYPE_FLOAT)
        {
            Linear_controls* controls =
                Device_impl_get_cv_float_controls_mut(dimpl, dstate, NULL, key);
            if (controls != NULL)
                Linear_controls_set_value(controls, value->value.float_type);
        }
        else
        {
            Device_impl_set_cv_generic(dimpl, dstate, NULL, key, value);
        }
    }
    else
    {
        assert(channel != NULL);
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
        {
            if (value->type == VALUE_TYPE_FLOAT)
            {
                Linear_controls* controls = Device_impl_get_cv_float_controls_mut(
                        dimpl, dstate, voice->state, key);
                if (controls != NULL)
                    Linear_controls_set_value(controls, value->value.float_type);
            }
            else
            {
                Device_impl_set_cv_generic(dimpl, dstate, voice->state, key, value);
            }
        }
    }

    return;
}


static Linear_controls* get_cv_float_controls(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);

    const Processor* proc = (const Processor*)device;
    const Device_impl* dimpl = device->dimpl;
    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    Linear_controls* controls = NULL;

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        controls = Device_impl_get_cv_float_controls_mut(dimpl, dstate, NULL, key);
    }
    else
    {
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            controls = Device_impl_get_cv_float_controls_mut(
                    dimpl, dstate, voice->state, key);
    }

    return controls;
}


static void Processor_slide_control_var_float_target(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double value)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(isfinite(value));

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_slide_value_target(controls, value);

    return;
}


static void Processor_slide_control_var_float_length(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_slide_value_length(controls, length);

    return;
}


static void Processor_osc_speed_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double speed)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(speed >= 0);

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_osc_speed_value(controls, speed);

    return;
}


static void Processor_osc_depth_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        double depth)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(isfinite(depth));

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_osc_depth_value(controls, depth);

    return;
}


static void Processor_osc_speed_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_osc_speed_slide_value(controls, length);

    return;
}


static void Processor_osc_depth_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Tstamp* length)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (controls != NULL)
        Linear_controls_osc_depth_slide_value(controls, length);

    return;
}


static void Processor_init_control_var_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* key,
        const Linear_controls* src_controls)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(channel != NULL);
    assert(key != NULL);
    assert(src_controls != NULL);

    Linear_controls* dest_controls =
        get_cv_float_controls(device, dstates, mode, channel, key);

    if (dest_controls != NULL)
        Linear_controls_copy(dest_controls, src_controls);

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


