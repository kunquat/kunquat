

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#include <Voice_state.h>
#include <Voice_params.h>
#include <Reltime.h>
#include <kunquat/limits.h>


Voice_state* Voice_state_init(Voice_state* state,
                              Voice_params* params,
                              uint32_t freq,
                              double tempo)
{
    assert(state != NULL);
    assert(params != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    Voice_state_clear(state);
    state->active = true;
    state->note_on = true;
    state->freq = freq;
    state->tempo = tempo;
    Voice_params_copy(&state->params, params);
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
    state->orig_cents = 0;

    state->pitch = 0;
    state->actual_pitch = 0;
    state->pitch_slide = 0;
    Reltime_init(&state->pitch_slide_length);
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

    state->autowah = false;
    state->autowah_length = 0;
    state->autowah_depth = 0;
    state->autowah_depth_target = 0;
    state->autowah_delay_pos = 0;
    state->autowah_delay_update = 1;
    state->autowah_phase = 0;
    state->autowah_update = 0;

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;
    
    state->pedal = NULL;
    state->fe_pos = 0;
    state->rel_fe_pos = 0;

    state->force = 1;
    state->actual_force = 1;
    state->force_slide = 0;
    Reltime_init(&state->force_slide_length);
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
    Reltime_init(&state->panning_slide_length);
    state->panning_slide_target = 0;
    state->panning_slide_frames = 0;
    state->panning_slide_update = 0;

    state->filter = INFINITY;
    state->actual_filter = INFINITY;
    state->effective_filter = INFINITY;
    state->filter_slide = 0;
    Reltime_init(&state->filter_slide_length);
    state->filter_slide_target = INFINITY;
    state->filter_slide_frames = 0;
    state->filter_slide_update = 0;
    state->filter_resonance = 1;
    state->effective_resonance = 1;
    state->filter_update = false;
    state->filter_state_used = -1;
    state->filter_xfade_state_used = -1;
    state->filter_xfade_pos = 1;
    state->filter_xfade_update = 0;
    for (int i = 0; i < FILTER_ORDER; ++i)
    {
        state->filter_state[0].coeffs1[i] = 0;
        state->filter_state[0].coeffs2[i] = 0;
        state->filter_state[1].coeffs1[i] = 0;
        state->filter_state[1].coeffs2[i] = 0;
        for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
        {
            state->filter_state[0].history1[k][i] = 0;
            state->filter_state[0].history2[k][i] = 0;
            state->filter_state[1].history1[k][i] = 0;
            state->filter_state[1].history2[k][i] = 0;
        }
    }
    state->filter_state[0].coeffs2[FILTER_ORDER] = 0;
    state->filter_state[1].coeffs2[FILTER_ORDER] = 0;

    return state;
}


