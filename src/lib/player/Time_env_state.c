

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Time_env_state.h>

#include <debug/assert.h>
#include <init/devices/param_types/Envelope.h>
#include <mathnum/common.h>
#include <mathnum/fast_exp2.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


void Time_env_state_init(Time_env_state* testate)
{
    assert(testate != NULL);

    testate->is_finished = false;
    testate->cur_pos = 0;
    testate->next_node_index = 0;
    testate->cur_value = NAN;
    testate->update_value = 0;
    testate->scale_factor = 1;

    return;
}


int32_t Time_env_state_process(
        Time_env_state* testate,
        const Envelope* env,
        bool has_loop,
        double scale_amount,
        double scale_center,
        double sustain,
        double min_value,
        double max_value,
        const Work_buffer* pitch_wb,
        float* env_buf,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate)
{
    assert(testate != NULL);
    assert(env != NULL);
    assert(isfinite(scale_amount));
    assert(isfinite(scale_center));
    assert(sustain >= 0);
    assert(sustain <= 1);
    assert(isfinite(min_value));
    assert(isfinite(max_value));
    assert(pitch_wb != NULL);
    assert(env_buf != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);

    if (testate->is_finished)
        return buf_start;

    const float* pitch_buf = Work_buffer_get_contents(pitch_wb);
    const int32_t pitch_const_start = Work_buffer_get_const_start(pitch_wb);
    double fixed_scale_factor = 1.0;
    if (pitch_const_start < buf_stop)
        fixed_scale_factor =
            exp2((pitch_buf[pitch_const_start] - scale_center) * scale_amount / 1200.0);

    // Get constant values used inside the loop
    const double slowdown_fac = 1.0 - sustain;
    const double inv_audio_rate = 1.0 / audio_rate;
    const double slowdown_fac_inv_audio_rate = slowdown_fac * inv_audio_rate;

    const double* last_node = Envelope_get_node(env, Envelope_node_count(env) - 1);

    // Get loop information
    const int loop_start_index = Envelope_get_mark(env, 0);
    const int loop_end_index = Envelope_get_mark(env, 1);
    const double* loop_start =
        (loop_start_index == -1) ? NULL : Envelope_get_node(env, loop_start_index);
    const double* loop_end =
        (loop_end_index == -1) ? NULL : Envelope_get_node(env, loop_end_index);
    if ((loop_start == NULL) || (loop_end == NULL))
        has_loop = false;

    // Get state variables
    double cur_pos = testate->cur_pos;
    int next_node_index = testate->next_node_index;
    double cur_value = testate->cur_value;
    double update_value = testate->update_value;
    double scale_factor = testate->scale_factor;

    double* next_node = Envelope_get_node(env, next_node_index);

    int32_t i = buf_start;
    for (; i < buf_stop; ++i)
    {
        const float pitch = pitch_buf[i];

        // Apply pitch-based scaling
        scale_factor = fixed_scale_factor;
        if (i < pitch_const_start)
            scale_factor = fast_exp2((pitch - scale_center) * scale_amount / 1200.0);

        // Get envelope value at current position
        double value = last_node[1]; // initial value is used if next_node == NULL

        if (next_node != NULL)
        {
            if (cur_pos < next_node[0])
            {
                // Get next value through calculated update value
                // (faster than accessing the envelope directly)
                assert(isfinite(update_value));
                cur_value += update_value * scale_factor * slowdown_fac;
                value = clamp(cur_value, min_value, max_value);
            }
            else
            {
                // Update value is obsolete, so use the envelope directly
                ++next_node_index;
                if ((loop_end_index >= 0) && (loop_end_index < next_node_index))
                {
                    assert(loop_start_index >= 0);
                    next_node_index = loop_start_index;
                }

                next_node = Envelope_get_node(env, next_node_index);

                value = Envelope_get_value(env, cur_pos);

                if (isfinite(value))
                {
                    // Get new update value
                    const double next_value = Envelope_get_value(
                            env, cur_pos + inv_audio_rate);
                    cur_value = value;

                    if (isfinite(next_value))
                        update_value = next_value - value;
                    else
                        value = last_node[1];
                }
                else
                {
                    // Reached the end of envelope
                    value = last_node[1];
                }
            }
        }

        // Apply envelope value
        assert(isfinite(value));
        env_buf[i] = (float)value;

        // Update envelope position
        double new_pos = cur_pos + scale_factor * slowdown_fac_inv_audio_rate;

        if (!has_loop)
        {
            // Check for end of envelope
            if (new_pos > last_node[0])
            {
                ++i;
                testate->is_finished = true;
                break;
            }
        }
        else
        {
            // Handle loop
            if (new_pos > loop_end[0])
            {
                const double loop_len = loop_end[0] - loop_start[0];
                assert(loop_len >= 0);
                new_pos = loop_end[0];

                if (loop_len > 0)
                {
                    const double exceed = new_pos - loop_end[0];
                    const double offset = fmod(exceed, loop_len);
                    new_pos = loop_start[0] + offset;
                    assert(new_pos >= loop_start[0]);
                    assert(new_pos <= loop_end[0]);

                    // Following iteration will check if this index is too low,
                    // so no need to find the exact result in a loop
                    next_node_index = loop_start_index;
                    next_node = Envelope_get_node(env, next_node_index);
                }
            }
        }

        cur_pos = new_pos;
    }

    // Update state for next process cycle
    testate->cur_pos = cur_pos;
    testate->next_node_index = next_node_index;
    testate->cur_value = cur_value;
    testate->update_value = update_value;
    testate->scale_factor = scale_factor;

    return i;
}


