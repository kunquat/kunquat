

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
#include <player/Slider.h>

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


Voice_state* Voice_state_init(Voice_state* state, Random* rand_p, Random* rand_s)
{
    rassert(state != NULL);
    rassert(rand_p != NULL);
    rassert(rand_s != NULL);

    Voice_state_clear(state);
    state->active = true;
    state->has_finished = false;
    state->note_on = true;
    state->rand_p = rand_p;
    state->rand_s = rand_s;

    state->render_voice = NULL;

    state->has_release_data = false;
    state->release_stop = 0;

    state->is_pitch_state = false;
    state->is_force_state = false;
    state->is_stream_state = false;

    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    rassert(state != NULL);

    state->active = false;
    state->has_finished = false;
    state->ramp_attack = 0;

    state->hit_index = -1;

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;

    state->is_pitch_state = false;
    state->is_force_state = false;
    state->is_stream_state = false;

    return state;
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
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

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

    // Call the implementation
    const int32_t impl_render_stop = vstate->render_voice(
            vstate, proc_state, au_state, wbs, buf_start, buf_stop, tempo);
    rassert(impl_render_stop <= buf_stop);

    return impl_render_stop;
}


void Voice_state_mix_signals(
        Voice_state* vstate,
        Proc_state* proc_state,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

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
    rassert(vstate != NULL);
    rassert(release_stop >= 0);

    vstate->has_release_data = true;
    vstate->release_stop = release_stop;

    return;
}


void Voice_state_set_finished(Voice_state* vstate)
{
    rassert(vstate != NULL);
    vstate->has_finished = true;
    return;
}


void Voice_state_cv_generic_set(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Value* value)
{
    rassert(vstate != NULL);
    rassert(dstate != NULL);
    rassert(key != NULL);
    rassert(value != NULL);

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
            cb->cb.set_float(vstate, dstate, cb->indices, value->value.float_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            cb->cb.set_tstamp(vstate, dstate, cb->indices, &value->value.Tstamp_type);
        }
        break;

        default:
            rassert(false);
    }

    return;
}


