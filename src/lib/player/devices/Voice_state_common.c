

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


#include <player/devices/Voice_state_common.h>

#include <debug/assert.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <player/Audio_buffer.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Pitch_controls.h>
#include <player/Work_buffers.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define RAMP_RELEASE_TIME (200.0)


void Voice_state_common_handle_pitch(
        Voice_state* vstate,
        const Processor* proc,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(vstate != NULL);
    assert(proc != NULL);
    assert(wbs != NULL);
    assert(buf_start < buf_stop);

    Pitch_controls* pc = &vstate->pitch_controls;

    float* pitch_params = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_PITCH_PARAMS);
    pitch_params[buf_start - 1] = pc->pitch;

    float* actual_pitches = Work_buffers_get_buffer_contents_mut(
            wbs, WORK_BUFFER_ACTUAL_PITCHES);
    actual_pitches[buf_start - 1] = vstate->actual_pitch;

    // Apply pitch slide
    if (Slider_in_progress(&pc->slider))
    {
        float new_pitch = pc->pitch;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_pitch = Slider_step(&pc->slider);
            pitch_params[i] = new_pitch;
        }
        pc->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitch_params[i] = pc->pitch;
    }

    // Adjust carried pitch
    if (pc->freq_mul != 1)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitch_params[i] *= pc->freq_mul;
    }

    // Initialise actual pitches
    memcpy(actual_pitches + buf_start,
            pitch_params + buf_start,
            sizeof(float) * (buf_stop - buf_start));

    // Apply vibrato
    if (LFO_active(&pc->vibrato))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            actual_pitches[i] *= LFO_step(&pc->vibrato);
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
            actual_pitches[i] *= diff;

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
    vstate->actual_pitch = actual_pitches[buf_stop - 1];
    vstate->prev_actual_pitch = actual_pitches[buf_stop - 2];

    return;
}


