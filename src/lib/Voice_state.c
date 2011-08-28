

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <Voice_state.h>
#include <Voice_params.h>
#include <Reltime.h>
#include <Slider.h>
#include <kunquat/limits.h>
#include <xassert.h>


Voice_state* Voice_state_init(Voice_state* state,
                              Voice_params* params,
                              Channel_gen_state* cgstate,
                              Random* rand_p,
                              Random* rand_s,
                              uint32_t freq,
                              double tempo)
{
    assert(state != NULL);
    assert(params != NULL);
    assert(cgstate != NULL);
    assert(rand_p != NULL);
    assert(rand_s != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    Voice_state_clear(state);
    state->cgstate = cgstate;
    state->active = true;
    state->note_on = true;
    state->freq = freq;
    state->tempo = tempo;
    state->rand_p = rand_p;
    state->rand_s = rand_s;

    Slider_set_mix_rate(&state->pitch_slider, freq);
    Slider_set_tempo(&state->pitch_slider, tempo);
    LFO_set_mix_rate(&state->vibrato, freq);
    LFO_set_tempo(&state->vibrato, tempo);
    Slider_set_mix_rate(&state->force_slider, freq);
    Slider_set_tempo(&state->force_slider, tempo);
    LFO_set_mix_rate(&state->tremolo, freq);
    LFO_set_tempo(&state->tremolo, tempo);
    Slider_set_mix_rate(&state->panning_slider, freq);
    Slider_set_tempo(&state->panning_slider, tempo);
    Slider_set_mix_rate(&state->lowpass_slider, freq);
    Slider_set_tempo(&state->lowpass_slider, tempo);
    LFO_set_mix_rate(&state->autowah, freq);
    LFO_set_tempo(&state->autowah, tempo);

    Voice_params_copy(&state->params, params);
    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);
    state->cgstate = NULL;

    state->active = false;
    state->freq = 0;
    state->tempo = 0;
    state->ramp_attack = 0;
    state->ramp_release = 0;
    state->orig_cents = 0;

    state->pitch = 0;
    state->prev_pitch = 0;
    state->actual_pitch = 0;
    state->prev_actual_pitch = 0;
    Slider_init(&state->pitch_slider, SLIDE_MODE_EXP);
    LFO_init(&state->vibrato, LFO_MODE_EXP);
    state->arpeggio = false;
    state->arpeggio_length = 0;
    state->arpeggio_frames = 0;
    state->arpeggio_note = 0;
    for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i)
    {
        state->arpeggio_factors[i] = 0;
    }

    LFO_init(&state->autowah, LFO_MODE_EXP);

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
    state->fe_next_node = 0;
    state->fe_value = NAN;
    state->fe_update = 0;
    state->fe_scale = 1;

    state->rel_fe_pos = 0;
    state->rel_fe_next_node = 0;
    state->rel_fe_value = NAN;
    state->rel_fe_update = 0;
    state->rel_fe_scale = NAN;

    state->force = 1;
    state->actual_force = 1;
    Slider_init(&state->force_slider, SLIDE_MODE_EXP);
    LFO_init(&state->tremolo, LFO_MODE_EXP);

    state->panning = 0;
    state->actual_panning = 0;
    Slider_init(&state->panning_slider, SLIDE_MODE_LINEAR);

    state->lowpass = INFINITY;
    state->actual_lowpass = INFINITY;
    state->effective_lowpass = INFINITY;
    Slider_init(&state->lowpass_slider, SLIDE_MODE_EXP);
    state->lowpass_resonance = 1;
    state->effective_resonance = 1;
    state->lowpass_update = false;
    state->lowpass_state_used = -1;
    state->lowpass_xfade_state_used = -1;
    state->lowpass_xfade_pos = 1;
    state->lowpass_xfade_update = 0;
    for (int i = 0; i < FILTER_ORDER; ++i)
    {
        state->lowpass_state[0].coeffs[i] = 0;
        state->lowpass_state[1].coeffs[i] = 0;
        for (int k = 0; k < KQT_BUFFERS_MAX; ++k)
        {
            state->lowpass_state[0].history1[k][i] = 0;
            state->lowpass_state[0].history2[k][i] = 0;
            state->lowpass_state[1].history1[k][i] = 0;
            state->lowpass_state[1].history2[k][i] = 0;
        }
    }

    return state;
}


