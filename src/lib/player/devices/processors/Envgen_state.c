

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Envgen_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_envgen.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <memory.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>
#include <string/common.h>
#include <Value.h>

#include <stdlib.h>
#include <string.h>


typedef struct Envgen_vstate
{
    Voice_state parent;

    float cur_y_min;
    float cur_y_max;
    bool activated;
    bool trig_floor_active;
    bool trig_ceil_active;
    bool was_released;
    Time_env_state env_state;
} Envgen_vstate;


int32_t Envgen_vstate_get_size(void)
{
    return sizeof(Envgen_vstate);
}


enum
{
    PORT_IN_STRETCH = 0,
    PORT_IN_TRIGGER,
    PORT_IN_COUNT,
};

enum
{
    PORT_OUT_ENV = 0,
    PORT_OUT_COUNT
};


static const int ENVGEN_WB_FIXED_STRETCH = WORK_BUFFER_IMPL_1;
static const int ENVGEN_WB_FIXED_TRIGGER = WORK_BUFFER_IMPL_2;


static void Envgen_state_set_cur_y_range(
        Envgen_vstate* egen_state, Random* random, const Proc_envgen* egen)
{
    rassert(egen_state != NULL);
    rassert(random != NULL);
    rassert(egen != NULL);

    if (egen->is_linear_force)
    {
        egen_state->cur_y_max =
            (float)(Random_get_float_signal(random) * egen->y_max_var);
        return;
    }

    const float min_y_allowed = (float)(egen->y_min - egen->y_min_var);
    const float max_y_allowed = (float)(egen->y_max + egen->y_max_var);

    float new_y_min =
        (float)(egen->y_min + Random_get_float_signal(random) * egen->y_min_var);
    new_y_min = min(new_y_min, max_y_allowed);

    float new_y_max =
        (float)(egen->y_max + Random_get_float_signal(random) * egen->y_max_var);
    new_y_max = max(new_y_max, min_y_allowed);

    if (new_y_min > new_y_max)
    {
        const float avg = (new_y_min + new_y_max) * 0.5f;
        const float clamped_avg = clamp(avg, min_y_allowed, max_y_allowed);
        new_y_min = clamped_avg;
        new_y_max = clamped_avg;
    }
    rassert(new_y_min <= new_y_max);

    egen_state->cur_y_min = new_y_min;
    egen_state->cur_y_max = new_y_max;

    return;
}


int32_t Envgen_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Proc_envgen* egen = (Proc_envgen*)proc_state->parent.device->dimpl;
    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    // Get time stretch input
    Work_buffer* stretch_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_STRETCH);
    if (!Work_buffer_is_valid(stretch_wb))
    {
        stretch_wb = Work_buffers_get_buffer_mut(wbs, ENVGEN_WB_FIXED_STRETCH);
        float* stretches = Work_buffer_get_contents_mut(stretch_wb);
        for (int32_t i = 0; i < frame_count; ++i)
            stretches[i] = 0;
        Work_buffer_set_const_start(stretch_wb, 0);
    }
    else
    {
        Proc_clamp_pitch_values(stretch_wb, 0, frame_count);
    }

    // Get trigger signal input
    const Work_buffer* trigger_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_TRIGGER);
    if (!Work_buffer_is_valid(trigger_wb))
    {
        Work_buffer* fixed_trigger_wb =
            Work_buffers_get_buffer_mut(wbs, ENVGEN_WB_FIXED_TRIGGER);
        Work_buffer_clear(fixed_trigger_wb, 0, frame_count);
        trigger_wb = fixed_trigger_wb;
    }

    // Get output buffer for writing
    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_ENV);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return 0;
    }
    float* out_buffer = Work_buffer_get_contents_mut(out_wb);

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);

    int32_t const_start = 0;

    if (is_time_env_enabled)
    {
        int32_t slice_start = 0;
        while (slice_start < frame_count)
        {
            int32_t slice_stop = frame_count;

            bool trigger_after = false;

            // Process trigger signal
            if (egen->trig_impulse_floor || egen->trig_impulse_ceil)
            {
                int32_t change_trig_floor_at = INT32_MAX;
                int32_t change_trig_ceil_at = INT32_MAX;

                // Process floor trigger
                if (egen->trig_impulse_floor)
                {
                    const float* trigger = Work_buffer_get_contents(trigger_wb);

                    if (egen_state->trig_floor_active)
                    {
                        // See how long we should continue in triggered state
                        const float off_level = egen->trig_impulse_floor_off;
                        for (int32_t on_i = slice_start; on_i < slice_stop; ++on_i)
                        {
                            if (trigger[on_i] > off_level)
                            {
                                change_trig_floor_at = on_i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // See how long we should continue outside triggered state
                        const float on_level = egen->trig_impulse_floor_on;
                        for (int32_t off_i = slice_start; off_i < slice_stop; ++off_i)
                        {
                            if (trigger[off_i] <= on_level)
                            {
                                change_trig_floor_at = off_i;
                                break;
                            }
                        }
                    }
                }

                // Process ceil trigger
                if (egen->trig_impulse_ceil)
                {
                    const float* trigger = Work_buffer_get_contents(trigger_wb);

                    if (egen_state->trig_ceil_active)
                    {
                        // See how long we should continue in triggered state
                        const float off_level = egen->trig_impulse_ceil_off;
                        for (int32_t on_i = slice_start; on_i < slice_stop; ++on_i)
                        {
                            if (trigger[on_i] < off_level)
                            {
                                change_trig_ceil_at = on_i;
                                break;
                            }
                        }
                    }
                    else
                    {
                        // See how long we should continue outside triggered state
                        const float on_level = egen->trig_impulse_ceil_on;
                        for (int32_t off_i = slice_start; off_i < slice_stop; ++off_i)
                        {
                            if (trigger[off_i] >= on_level)
                            {
                                change_trig_ceil_at = off_i;
                                break;
                            }
                        }
                    }
                }

                if (change_trig_floor_at < change_trig_ceil_at)
                {
                    // Change floor trigger status after this slice
                    rassert(change_trig_floor_at < slice_stop);
                    slice_stop = change_trig_floor_at;

                    egen_state->trig_floor_active = !egen_state->trig_floor_active;
                    if (egen_state->trig_floor_active)
                        trigger_after = true;
                }
                else if (change_trig_ceil_at < change_trig_floor_at)
                {
                    // Change ceil trigger status after this slice
                    rassert(change_trig_ceil_at < slice_stop);
                    slice_stop = change_trig_ceil_at;

                    egen_state->trig_ceil_active = !egen_state->trig_ceil_active;
                    if (egen_state->trig_ceil_active)
                        trigger_after = true;
                }
                else if (change_trig_ceil_at < slice_stop)
                {
                    // Change both trigger statuses after this slice
                    rassert(change_trig_ceil_at == change_trig_floor_at);
                    slice_stop = change_trig_ceil_at;

                    egen_state->trig_floor_active = !egen_state->trig_floor_active;
                    egen_state->trig_ceil_active = !egen_state->trig_ceil_active;
                    if (egen_state->trig_floor_active || egen_state->trig_ceil_active)
                        trigger_after = true;
                }
            }

            // Process release trigger
            if (egen->trig_release)
            {
                if (!egen_state->was_released && !vstate->note_on)
                {
                    egen_state->was_released = true;

                    Envgen_state_set_cur_y_range(egen_state, vstate->rand_p, egen);
                    Time_env_state_init(&egen_state->env_state);
                    egen_state->activated = true;
                }
            }

            // Process envelope
            if (!egen_state->activated)
            {
                // Apply the start of the envelope before activated for the first time
                const double* first_node = Envelope_get_node(egen->time_env, 0);
                const float first_val = (float)first_node[1];

                for (int32_t i = slice_start; i < slice_stop; ++i)
                    out_buffer[i] = first_val;

                // Note: zero is correct here as we never return to inactive state
                const_start = 0;
            }
            else
            {
                // Apply the time envelope
                const int32_t env_stop = Time_env_state_process(
                        &egen_state->env_state,
                        egen->time_env,
                        egen->is_loop_enabled,
                        0, // sustain
                        0, 1, // range, NOTE: this is mapped to [y_min, y_max] later
                        stretch_wb,
                        Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                        slice_start,
                        slice_stop,
                        proc_state->parent.audio_rate);

                float* time_env =
                    Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV);

                // Check the end of envelope processing
                if (egen_state->env_state.is_finished)
                {
                    const double* last_node = Envelope_get_node(
                            egen->time_env, Envelope_node_count(egen->time_env) - 1);
                    const float last_value = (float)last_node[1];

                    // Fill the rest of the envelope buffer with the last value
                    for (int32_t i = env_stop; i < slice_stop; ++i)
                        time_env[i] = last_value;

                    const bool is_final =
                        !egen->trig_impulse_floor &&
                        !egen->trig_impulse_ceil &&
                        (!egen->trig_release || egen_state->was_released);

                    Work_buffer_set_final(out_wb, is_final);
                }

                const_start = env_stop;

                // Write to signal output
                for (int32_t i = slice_start; i < slice_stop; ++i)
                    out_buffer[i] = time_env[i];
            }

            // Scale and write to output
            if (egen->is_linear_force)
            {
                // Convert to dB
                const double global_adjust = egen->global_adjust;

                const float add = (float)global_adjust + egen_state->cur_y_max;

                const int32_t fast_stop = min(const_start, slice_stop);

                for (int32_t i = slice_start; i < fast_stop; ++i)
                    out_buffer[i] = (float)fast_scale_to_dB(out_buffer[i]) + add;

                if (fast_stop < slice_stop)
                {
                    const float dB = (float)scale_to_dB(out_buffer[fast_stop]) + add;
                    for (int32_t i = fast_stop; i < slice_stop; ++i)
                        out_buffer[i] = dB;
                }
            }
            else
            {
                const double range_width = egen_state->cur_y_max - egen_state->cur_y_min;

                if ((egen_state->cur_y_min != 0) || (egen_state->cur_y_max != 1))
                {
                    // Apply range
                    for (int32_t i = slice_start; i < slice_stop; ++i)
                        out_buffer[i] =
                            (float)(egen_state->cur_y_min + out_buffer[i] * range_width);
                }

                const float global_adjust = (float)egen->global_adjust;
                for (int32_t i = slice_start; i < slice_stop; ++i)
                    out_buffer[i] += global_adjust;
            }

            if (trigger_after)
            {
                Envgen_state_set_cur_y_range(egen_state, vstate->rand_p, egen);
                Time_env_state_init(&egen_state->env_state);
                egen_state->activated = true;
            }

            slice_start = slice_stop;
        }
    }
    else
    {
        // Initialise with default values
        const float value = egen_state->cur_y_max;
        for (int32_t i = 0; i < frame_count; ++i)
            out_buffer[i] = value;

        Work_buffer_set_final(out_wb, true);
    }

    // Mark constant region of the buffer
    Work_buffer_set_const_start(out_wb, const_start);

    return frame_count;
}


void Envgen_vstate_fire_event(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* event_name,
        const Value* arg)
{
    rassert(vstate != NULL);
    rassert(dstate != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);

    if ((arg->type == VALUE_TYPE_NONE) && string_eq(event_name, "trigger"))
    {
        const Proc_envgen* egen = (const Proc_envgen*)dstate->device->dimpl;

        const bool is_time_env_enabled =
            egen->is_time_env_enabled && (egen->time_env != NULL);

        if (is_time_env_enabled)
        {
            Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

            Envgen_state_set_cur_y_range(egen_state, vstate->rand_p, egen);
            Time_env_state_init(&egen_state->env_state);
            egen_state->activated = true;
        }
    }

    return;
}


void Envgen_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    const Device_state* dstate = (const Device_state*)proc_state;
    const Proc_envgen* egen = (const Proc_envgen*)dstate->device->dimpl;

    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    egen_state->cur_y_min = 0;
    egen_state->cur_y_max = 1;
    egen_state->activated = egen->trig_immediate;
    egen_state->trig_floor_active = false;
    egen_state->trig_ceil_active = false;
    egen_state->was_released = false;

    Envgen_state_set_cur_y_range(egen_state, vstate->rand_p, egen);
    Time_env_state_init(&egen_state->env_state);

    return;
}


