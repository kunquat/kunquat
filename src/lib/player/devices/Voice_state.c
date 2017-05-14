

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
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
#include <init/devices/Au_expressions.h>
#include <init/devices/Audio_unit.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Param_proc_filter.h>
#include <init/devices/Proc_type.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <player/Device_states.h>
#include <player/devices/Device_thread_state.h>
#include <player/Slider.h>

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Voice_state* Voice_state_init(
        Voice_state* state, Proc_type proc_type, Random* rand_p, Random* rand_s)
{
    rassert(state != NULL);
    rassert(proc_type >= 0);
    rassert(proc_type < Proc_type_COUNT);
    rassert(rand_p != NULL);
    rassert(rand_s != NULL);

    Voice_state_clear(state);

    state->proc_type = proc_type;

    state->active = true;
    state->keep_alive_stop = 0;
    state->note_on = true;
    state->rand_p = rand_p;
    state->rand_s = rand_s;
    state->wb = NULL;

    state->render_voice = NULL;

    return state;
}


void Voice_state_set_work_buffer(Voice_state* state, Work_buffer* wb)
{
    rassert(state != NULL);
    state->wb = wb;
    return;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    rassert(state != NULL);

    state->proc_type = Proc_type_COUNT;

    state->active = false;
    state->keep_alive_stop = 0;
    state->ramp_attack = 0;

    state->expr_filters_applied = false;
    memset(state->ch_expr_name, '\0', KQT_VAR_NAME_MAX);
    memset(state->note_expr_name, '\0', KQT_VAR_NAME_MAX);

    memset(state->test_proc_param, '\0', KQT_VAR_NAME_MAX);

    state->hit_index = -1;

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;

    return state;
}


static bool is_proc_filtered(
        const Processor* proc, const Au_expressions* ae, const char* expr_name)
{
    rassert(proc != NULL);
    rassert(ae != NULL);
    rassert(expr_name != NULL);

    if (expr_name[0] == '\0')
        return false;

    const Param_proc_filter* proc_filter = Au_expressions_get_proc_filter(ae, expr_name);
    if (proc_filter == NULL)
        return false;

    return !Param_proc_filter_is_proc_allowed(proc_filter, proc->index);
}


int32_t Voice_state_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;
    if (!Processor_get_voice_signals(proc) || (vstate->render_voice == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    if (!vstate->expr_filters_applied)
    {
        // Stop processing if we are filtered out by current Audio unit expressions
        const Audio_unit* au = (const Audio_unit*)au_state->parent.device;
        const Au_expressions* ae = Audio_unit_get_expressions(au);
        if (ae != NULL)
        {
            if (is_proc_filtered(proc, ae, vstate->ch_expr_name) ||
                    is_proc_filtered(proc, ae, vstate->note_expr_name))
            {
                vstate->active = false;
                return buf_start;
            }
        }

        vstate->expr_filters_applied = true;
    }

    if (buf_start >= buf_stop)
        return buf_start;

    // Call the implementation
    const int32_t impl_render_stop = vstate->render_voice(
            vstate, proc_state, proc_ts, au_state, wbs, buf_start, buf_stop, tempo);
    rassert(impl_render_stop <= buf_stop);

    return impl_render_stop;
}


void Voice_state_mix_signals(
        Voice_state* vstate,
        Proc_state* proc_state,
        Device_thread_state* proc_ts,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    for (int32_t port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* mixed_buffer =
            Device_thread_state_get_mixed_buffer(proc_ts, DEVICE_PORT_TYPE_SEND, port);
        if (mixed_buffer == NULL)
            continue;

        const Work_buffer* voice_buffer =
            Device_thread_state_get_voice_buffer(proc_ts, DEVICE_PORT_TYPE_SEND, port);
        rassert(voice_buffer != NULL);

        Work_buffer_mix(mixed_buffer, voice_buffer, buf_start, buf_stop);
        Device_thread_state_mark_mixed_audio(proc_ts);
    }

    return;
}


void Voice_state_set_keep_alive_stop(Voice_state* vstate, int32_t stop)
{
    rassert(vstate != NULL);
    rassert(stop >= 0);

    vstate->keep_alive_stop = stop;

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


