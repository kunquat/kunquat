

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/Slider.h>
#include <player/Voice_state.h>
#include <Tstamp.h>


Voice_state* Voice_state_init(
        Voice_state* state,
        Channel_proc_state* cpstate,
        Random* rand_p,
        Random* rand_s,
        uint32_t freq,
        double tempo)
{
    assert(state != NULL);
    assert(cpstate != NULL);
    assert(rand_p != NULL);
    assert(rand_s != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    Voice_state_clear(state);
    state->cpstate = cpstate;
    state->active = true;
    state->note_on = true;
    state->freq = freq;
    state->tempo = tempo;
    state->rand_p = rand_p;
    state->rand_s = rand_s;

    Force_controls_init(&state->force_controls, freq, tempo);
    Pitch_controls_init(&state->pitch_controls, freq, tempo);

#define SET_RATE_TEMPO(type, field) \
    type ## _set_mix_rate(&state->field, freq); \
    type ## _set_tempo(&state->field, tempo)

    SET_RATE_TEMPO(Slider, panning_slider);
    SET_RATE_TEMPO(Slider, lowpass_slider);
    SET_RATE_TEMPO(LFO, autowah);
    SET_RATE_TEMPO(Slider, lowpass_resonance_slider);

#undef SET_RATE_TEMPO

    return state;
}


Voice_state* Voice_state_clear(Voice_state* state)
{
    assert(state != NULL);
    state->cpstate = NULL;

    state->active = false;
    state->freq = 0;
    state->tempo = 0;
    state->ramp_attack = 0;
    state->ramp_release = 0;

    state->hit_index = -1;
    Pitch_controls_reset(&state->pitch_controls);
    state->actual_pitch = 0;
    state->prev_actual_pitch = 0;

    state->arpeggio = false;
    state->arpeggio_ref = NAN;
    state->arpeggio_length = 0;
    state->arpeggio_frames = 0;
    state->arpeggio_note = 0;
    state->arpeggio_tones[0] = state->arpeggio_tones[1] = NAN;
#if 0
    for (int i = 0; i < KQT_ARPEGGIO_NOTES_MAX; ++i)
    {
        state->arpeggio_offsets[i] = NAN;
    }
#endif

    LFO_init(&state->autowah, LFO_MODE_LINEAR);

    state->pos = 0;
    state->pos_rem = 0;
    state->rel_pos = 0;
    state->rel_pos_rem = 0;
    state->dir = 1;
    state->note_on = false;
    state->noff_pos = 0;
    state->noff_pos_rem = 0;

    Time_env_state_init(&state->force_env_state);
    Time_env_state_init(&state->force_rel_env_state);

    Force_controls_reset(&state->force_controls);
    state->actual_force = 1;

    state->panning = 0;
    state->actual_panning = 0;
    Slider_init(&state->panning_slider, SLIDE_MODE_LINEAR);

    state->pitch_pan_ref_param = FLT_MAX;
    state->pitch_pan_value = 0;

    Time_env_state_init(&state->env_filter_state);
    Time_env_state_init(&state->env_filter_rel_state);

    state->lowpass = 100;
    state->actual_lowpass = state->lowpass;
    state->lowpass_resonance = 0;

    state->applied_lowpass = state->actual_lowpass;
    state->applied_resonance = state->lowpass_resonance;
    state->true_lowpass = INFINITY;
    Slider_init(&state->lowpass_slider, SLIDE_MODE_LINEAR);
    Slider_init(&state->lowpass_resonance_slider, SLIDE_MODE_LINEAR);
    state->true_resonance = 0.5;
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


