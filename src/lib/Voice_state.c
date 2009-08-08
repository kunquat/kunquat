

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#include <Voice_state.h>
#include <kunquat/limits.h>


Voice_state* Voice_state_init(Voice_state* state,
                              const Channel_state* cur_ch_state,
                              Channel_state* new_ch_state,
                              uint32_t freq,
                              double tempo)
{
    assert(state != NULL);
    assert(cur_ch_state != NULL);
    assert(new_ch_state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    Voice_state_clear(state);
    state->active = true;
    state->note_on = true;
    state->cur_ch_state = cur_ch_state;
    state->new_ch_state = new_ch_state;
    state->freq = freq;
    state->tempo = tempo;
    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);
    state->active = false;
    state->freq = 0;
    state->tempo = 0;
    state->ramp_attack = 0;
    state->ramp_release = 0;
    state->orig_note = 0;
    state->orig_note_mod = -1;
    state->orig_octave = KQT_SCALE_MIDDLE_OCTAVE;

    state->pitch = 0;
    state->actual_pitch = 0;
    state->pitch_slide = 0;
    state->pitch_slide_target = 0;
    state->pitch_slide_frames = 0;
    state->pitch_slide_update = 1;
    state->vibrato = false;
    state->vibrato_length = 0;
    state->vibrato_depth = 0;
    state->vibrato_depth_target = 0;
    state->vibrato_delay_pos = 0;
    state->vibrato_delay_update = 1;
    state->vibrato_phase = 0;
    state->vibrato_update = 0;
    state->arpeggio = false;
    state->arpeggio_length = 0;
    state->arpeggio_frames = 0;
    state->arpeggio_note = 0;
    for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i)
    {
        state->arpeggio_factors[i] = 0;
    }

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;
    
    state->pedal = false;
    state->on_ve_pos = 0;
    state->off_ve_pos = 0;

    state->force = 1;
    state->actual_force = 1;
    state->force_slide = 0;
    state->force_slide_target = 1;
    state->force_slide_frames = 0;
    state->force_slide_update = 1;
    state->tremolo = false;
    state->tremolo_length = 0;
    state->tremolo_depth = 0;
    state->tremolo_depth_target = 0;
    state->tremolo_delay_pos = 0;
    state->tremolo_delay_update = 1;
    state->tremolo_phase = 0;
    state->tremolo_update = 0;

    state->panning = 0;
    state->actual_panning = 0;
    state->panning_slide = 0;
    state->panning_slide_target = 0;
    state->panning_slide_frames = 0;
    state->panning_slide_update = 0;

    state->filter = INFINITY;
    state->actual_filter = INFINITY;
    state->filter_update = false;
    for (int i = 0; i < FILTER_ORDER; ++i)
    {
        state->filter_coeffs1[i] = 0;
        state->filter_coeffs2[i] = 0;
        for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
        {
            state->filter_history1[k][i] = 0;
            state->filter_history2[k][i] = 0;
        }
    }
    state->filter_coeffs2[FILTER_ORDER] = 0;

    return state;
}


