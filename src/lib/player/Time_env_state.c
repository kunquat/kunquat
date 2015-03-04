

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


#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <devices/param_types/Envelope.h>
#include <mathnum/common.h>
#include <player/Time_env_state.h>
#include <player/Work_buffers.h>


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
        double scale_amount,
        double scale_center,
        double sustain,
        double min_value,
        double max_value,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate)
{
    assert(testate != NULL);
    assert(env != NULL);
    assert(isfinite(scale_amount));
    assert(isfinite(scale_center));
    assert(sustain >= 0);
    assert(sustain <= 1);
    assert(isfinite(min_value));
    assert(isfinite(max_value));
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);

    const float* actual_pitches = Work_buffers_get_buffer_contents(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);

    float* values = Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV);

    const double slowdown_fac = 1.0 - sustain;

    const double* last_node = Envelope_get_node(env, Envelope_node_count(env) - 1);

    // Get loop information
    const int loop_start_index = Envelope_get_mark(env, 0);
    const int loop_end_index = Envelope_get_mark(env, 1);
    const double* loop_start =
        (loop_start_index == -1) ? NULL : Envelope_get_node(env, loop_start_index);
    const double* loop_end =
        (loop_end_index == -1) ? NULL : Envelope_get_node(env, loop_end_index);

    const bool has_loop = (loop_start != NULL) && (loop_end != NULL);

    int32_t i = buf_start;
    for (; i < buf_stop; ++i)
    {
        const float actual_pitch = actual_pitches[i];
        const float prev_actual_pitch = actual_pitches[i - 1];

        // Apply pitch-based scaling
        if ((scale_amount != 0) && (actual_pitch != prev_actual_pitch))
            testate->scale_factor = pow(
                    actual_pitch / scale_center, scale_amount);

        // Get envelope value at current position
        double* next_node = Envelope_get_node(env, testate->next_node_index);
        double value = last_node[1]; // initial value is used if next_node == NULL

        if (next_node != NULL)
        {
            if (testate->cur_pos < next_node[0])
            {
                assert(isfinite(testate->update_value));
                testate->cur_value +=
                    testate->update_value * testate->scale_factor * slowdown_fac;
                value = clamp(testate->cur_value, min_value, max_value);
            }
            else
            {
                ++testate->next_node_index;
                if ((loop_end_index >= 0) && (loop_end_index < testate->next_node_index))
                {
                    assert(loop_start_index >= 0);
                    testate->next_node_index = loop_start_index;
                }

                value = Envelope_get_value(env, testate->cur_pos);

                if (isfinite(value))
                {
                    const double next_value = Envelope_get_value(
                            env, testate->cur_pos + 1.0 / audio_rate);
                    testate->cur_value = value;
                    testate->update_value = next_value - value;
                }
                else
                {
                    value = last_node[1];
                }
            }
        }

        // Apply envelope value
        assert(isfinite(value));
        values[i] = value;

        // Update envelope position
        double new_pos =
            testate->cur_pos + testate->scale_factor * slowdown_fac / audio_rate;

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
                    testate->next_node_index = loop_start_index;
                }
            }
        }

        testate->cur_pos = new_pos;
    }

    return i;
}


