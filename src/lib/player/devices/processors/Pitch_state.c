

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
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
#include <player/Pitch_controls.h>

#include <stdlib.h>


typedef struct Pitch_vstate
{
    Voice_state parent;
} Pitch_vstate;


size_t Pitch_vstate_get_size(void)
{
    return sizeof(Pitch_vstate);
}


static int32_t Pitch_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get output
    float* out_buf =
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    Pitch_controls* pc = &vstate->pitch_controls;
    Pitch_controls_set_tempo(pc, tempo);

    out_buf[buf_start - 1] = pc->pitch;

    // Apply pitch slide
    if (Slider_in_progress(&pc->slider))
    {
        float new_pitch = pc->pitch;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_pitch = Slider_step(&pc->slider);
            out_buf[i] = new_pitch;
        }
        pc->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] = pc->pitch;
    }

    // Adjust carried pitch
    if (pc->freq_mul != 1)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] *= pc->freq_mul;
    }

    out_buf[buf_start - 1] = vstate->actual_pitch;

    // Apply vibrato
    if (LFO_active(&pc->vibrato))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] *= LFO_step(&pc->vibrato);
    }

    // Apply arpeggio
    if (vstate->arpeggio)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            // Adjust actual pitch according to the current arpeggio state
            assert(!isnan(vstate->arpeggio_tones[0]));
            double diff = exp2(
                    (vstate->arpeggio_tones[vstate->arpeggio_note] -
                        vstate->arpeggio_ref) / 1200);
            out_buf[i] *= diff;

            // Update arpeggio state
            vstate->arpeggio_frames += 1;
            if (vstate->arpeggio_frames >= vstate->arpeggio_length)
            {
                vstate->arpeggio_frames -= vstate->arpeggio_length;
                ++vstate->arpeggio_note;
                if (vstate->arpeggio_note > KQT_ARPEGGIO_NOTES_MAX ||
                        isnan(vstate->arpeggio_tones[vstate->arpeggio_note]))
                    vstate->arpeggio_note = 0;
            }
        }
    }

    // Update actual pitch for next iteration
    vstate->actual_pitch = out_buf[buf_stop - 1];
    vstate->prev_actual_pitch = out_buf[buf_stop - 2];

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Pitch_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Pitch_vstate_render_voice;

    return;
}


