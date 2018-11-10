

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
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

    bool is_arpeggio_enabled;
    double arpeggio_ref_pitch;
    double arpeggio_speed;
    double arpeggio_tone_progress;
    int arpeggio_tone_index;
    double arpeggio_tones[KQT_ARPEGGIO_TONES_MAX];
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
    rassert(buf_stop >= 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    const Device_state* dstate = &proc_state->parent;

    // Get output
    Work_buffer* out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_PITCH, NULL);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return buf_start;
    }
    float* out_buf = Work_buffer_get_contents_mut(out_wb, 0);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    Pitch_controls* pc = &pvstate->controls;
    if (!isfinite(pc->pitch))
    {
        vstate->active = false;
        return buf_start;
    }

    Pitch_controls_set_audio_rate(pc, dstate->audio_rate);
    Pitch_controls_set_tempo(pc, tempo);

    out_buf[buf_start - 1] = (float)pc->pitch;

    int32_t const_start = buf_start;

    // Apply pitch slide
    {
        int32_t cur_pos = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps =
                Slider_estimate_active_steps_left(&pc->slider);
            if (estimated_steps > 0)
            {
                int32_t slide_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
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
                for (int32_t i = cur_pos; i < buf_stop; ++i)
                    out_buf[i] = pitch;

                cur_pos = buf_stop;
            }
        }
    }

    // Adjust carried pitch
    if (pc->pitch_add != 0)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] += (float)pc->pitch_add;
    }

    // Apply vibrato
    {
        int32_t cur_pos = buf_start;
        int32_t final_lfo_stop = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps =
                LFO_estimate_active_steps_left(&pc->vibrato);
            if (estimated_steps > 0)
            {
                int32_t lfo_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
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

    // Apply arpeggio
    if (pvstate->is_arpeggio_enabled)
    {
        const_start = buf_stop;

        const double progress_update =
            (pvstate->arpeggio_speed / dstate->audio_rate) * (tempo / 60.0);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            // Adjust actual pitch according to the current arpeggio state
            rassert(!isnan(pvstate->arpeggio_tones[0]));
            const double diff = (pvstate->arpeggio_tones[pvstate->arpeggio_tone_index] -
                    pvstate->arpeggio_ref_pitch);
            out_buf[i] += (float)diff;

            // Update arpeggio state
            pvstate->arpeggio_tone_progress += progress_update;
            while (pvstate->arpeggio_tone_progress > 1.0)
            {
                pvstate->arpeggio_tone_progress -= 1.0;
                ++pvstate->arpeggio_tone_index;
                if (pvstate->arpeggio_tone_index >= KQT_ARPEGGIO_TONES_MAX ||
                        isnan(pvstate->arpeggio_tones[pvstate->arpeggio_tone_index]))
                    pvstate->arpeggio_tone_index = 0;
            }
        }
    }

    // Update pitch for next iteration
    pvstate->pitch = out_buf[buf_stop - 1];

    // Mark constant region of the buffer
    Work_buffer_set_const_start(out_wb, 0, const_start);

    return buf_stop;
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

    // Initialise arpeggio
    pvstate->is_arpeggio_enabled = false;
    pvstate->arpeggio_ref_pitch = NAN;
    pvstate->arpeggio_speed = 24;
    pvstate->arpeggio_tone_progress = 0;
    pvstate->arpeggio_tone_index = 0;
    pvstate->arpeggio_tones[0] = pvstate->arpeggio_tones[1] = NAN;

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


void Pitch_vstate_arpeggio_on(
        Voice_state* vstate,
        double speed,
        double ref_pitch,
        double tones[KQT_ARPEGGIO_TONES_MAX])
{
    rassert(vstate != NULL);
    rassert(isfinite(speed));
    rassert(speed > 0);
    rassert(isfinite(ref_pitch));
    rassert(tones != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    if (pvstate->is_arpeggio_enabled)
        return;

    pvstate->arpeggio_speed = speed;
    pvstate->arpeggio_ref_pitch = ref_pitch;
    memcpy(pvstate->arpeggio_tones, tones, KQT_ARPEGGIO_TONES_MAX * sizeof(double));
    pvstate->arpeggio_tone_index = 0;
    pvstate->is_arpeggio_enabled = true;

    return;
}


void Pitch_vstate_arpeggio_off(Voice_state* vstate)
{
    rassert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->is_arpeggio_enabled = false;

    return;
}


void Pitch_vstate_update_arpeggio_tones(
        Voice_state* vstate, double tones[KQT_ARPEGGIO_TONES_MAX])
{
    rassert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    memcpy(pvstate->arpeggio_tones, tones, KQT_ARPEGGIO_TONES_MAX * sizeof(double));

    return;
}


void Pitch_vstate_update_arpeggio_speed(Voice_state* vstate, double speed)
{
    rassert(vstate != NULL);
    rassert(isfinite(speed));
    rassert(speed > 0);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->arpeggio_speed = speed;

    return;
}


void Pitch_vstate_reset_arpeggio(Voice_state* vstate)
{
    rassert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->arpeggio_tones[0] = pvstate->arpeggio_tones[1] = NAN;
    pvstate->arpeggio_tone_index = 0;

    return;
}


