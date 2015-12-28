

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
#include <devices/Proc_type.h>
#include <Filter.h>
#include <memory.h>
#include <player/Channel.h>
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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_generic_set(dstate, key, value);
    }
    else
    {
        assert(channel != NULL);
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_generic_set(voice->state, dstate, key, value);
    }

    return;
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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_slide_target(dstate, key, value);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_slide_target(voice->state, dstate, key, value);
    }

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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_slide_length(dstate, key, length);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_slide_length(voice->state, dstate, key, length);
    }

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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_osc_speed(dstate, key, speed);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_osc_speed(voice->state, dstate, key, speed);
    }

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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_osc_depth(dstate, key, depth);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_osc_depth(voice->state, dstate, key, depth);
    }

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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_osc_speed_slide(dstate, key, length);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_osc_speed_slide(voice->state, dstate, key, length);
    }

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

    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));

    if (mode == DEVICE_CONTROL_VAR_MODE_MIXED)
    {
        Proc_state_cv_float_osc_depth_slide(dstate, key, length);
    }
    else
    {
        const Processor* proc = (const Processor*)device;
        Voice* voice = Channel_get_fg_voice(channel, proc->index);
        if (voice != NULL)
            Voice_state_cv_float_osc_depth_slide(voice->state, dstate, key, length);
    }

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
    assert(mode == DEVICE_CONTROL_VAR_MODE_VOICE);
    assert(channel != NULL);
    assert(key != NULL);
    assert(src_controls != NULL);

    const Processor* proc = (const Processor*)device;
    Device_state* dstate = Device_states_get_state(dstates, Device_get_id(device));
    Voice* voice = Channel_get_fg_voice(channel, proc->index);
    if (voice != NULL)
        Voice_state_cv_float_init(voice->state, dstate, key, src_controls);

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


