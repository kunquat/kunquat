

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Pitch_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_pitch.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <player/devices/Device_thread_state.h>
#include <player/Pitch_controls.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Pitch_vstate
{
    Voice_state parent;

    Pitch_controls controls;
    double orig_pitch_param;
    double pitch;
} Pitch_vstate;


int32_t Pitch_vstate_get_size(void)
{
    return sizeof(Pitch_vstate);
}


enum
{
    PORT_OUT_PITCH = 0,
    PORT_OUT_COUNT
};


int32_t Pitch_vstate_render_voice(
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

    const Device_state* dstate = &proc_state->parent;

    // Get output
    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_PITCH);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return 0;
    }
    float* out_buf = Work_buffer_get_contents_mut(out_wb);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    Pitch_controls* pc = &pvstate->controls;
    if (!isfinite(pc->pitch))
    {
        vstate->active = false;
        Work_buffer_invalidate(out_wb);
        return 0;
    }

    Pitch_controls_set_audio_rate(pc, dstate->audio_rate);
    Pitch_controls_set_tempo(pc, tempo);

    int32_t const_start = 0;

    // Apply pitch slide
    {
        int32_t cur_pos = 0;
        while (cur_pos < frame_count)
        {
            const int32_t estimated_steps =
                Slider_estimate_active_steps_left(&pc->slider);
            if (estimated_steps > 0)
            {
                int32_t slide_stop = frame_count;
                if (estimated_steps < frame_count - cur_pos)
                    slide_stop = cur_pos + estimated_steps;

                double new_pitch = pc->pitch;
                for (int32_t i = cur_pos; i < slide_stop; ++i)
                {
                    new_pitch = Slider_step(&pc->slider);
                    out_buf[i] = (float)new_pitch;
                }
                pc->pitch = new_pitch;

                const_start = slide_stop;
                cur_pos = slide_stop;
            }
            else
            {
                const float pitch = (float)pc->pitch;
                for (int32_t i = cur_pos; i < frame_count; ++i)
                    out_buf[i] = pitch;

                cur_pos = frame_count;
            }
        }
    }

    // Adjust carried pitch
    if (pc->pitch_add != 0)
    {
        for (int32_t i = 0; i < frame_count; ++i)
            out_buf[i] += (float)pc->pitch_add;
    }

    // Apply vibrato
    {
        int32_t cur_pos = 0;
        int32_t final_lfo_stop = 0;
        while (cur_pos < frame_count)
        {
            const int32_t estimated_steps =
                LFO_estimate_active_steps_left(&pc->vibrato);
            if (estimated_steps > 0)
            {
                int32_t lfo_stop = frame_count;
                if (estimated_steps < frame_count - cur_pos)
                    lfo_stop = cur_pos + estimated_steps;

                for (int32_t i = cur_pos; i < lfo_stop; ++i)
                    out_buf[i] += (float)LFO_step(&pc->vibrato);

                final_lfo_stop = lfo_stop;
                cur_pos = lfo_stop;
            }
            else
            {
                final_lfo_stop = cur_pos;
                break;
            }
        }

        const_start = max(const_start, final_lfo_stop);
    }

    // Update pitch for next iteration
    pvstate->pitch = out_buf[frame_count - 1];

    // Mark constant region of the buffer
    Work_buffer_set_const_start(out_wb, const_start);

    return frame_count;
}


void Pitch_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    // Initialise common pitch state
    Pitch_controls_init(&pvstate->controls, proc_state->parent.audio_rate, 120);

    pvstate->orig_pitch_param = NAN;
    pvstate->pitch = 0;

    return;
}


void Pitch_vstate_set_controls(Voice_state* vstate, const Pitch_controls* controls)
{
    rassert(vstate != NULL);
    rassert(controls != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    Pitch_controls_copy(&pvstate->controls, controls);

    if (isnan(pvstate->orig_pitch_param))
        pvstate->orig_pitch_param = pvstate->controls.orig_carried_pitch;

    pvstate->pitch = pvstate->controls.pitch;

    return;
}


