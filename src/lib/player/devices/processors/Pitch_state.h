

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PITCH_STATE_H
#define KQT_PITCH_STATE_H


#include <decl.h>
#include <kunquat/limits.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Pitch_controls.h>


Voice_state_get_size_func Pitch_vstate_get_size;
Voice_state_init_func Pitch_vstate_init;
Voice_state_render_voice_func Pitch_vstate_render_voice;

void Pitch_vstate_set_controls(Voice_state* vstate, const Pitch_controls* controls);

void Pitch_vstate_arpeggio_on(
        Voice_state* vstate,
        double speed,
        double ref_pitch,
        double tones[KQT_ARPEGGIO_TONES_MAX]);

void Pitch_vstate_arpeggio_off(Voice_state* vstate);

void Pitch_vstate_update_arpeggio_tones(
        Voice_state* vstate, double tones[KQT_ARPEGGIO_TONES_MAX]);

void Pitch_vstate_update_arpeggio_speed(Voice_state* vstate, double speed);

void Pitch_vstate_reset_arpeggio(Voice_state* vstate);


#endif // KQT_PITCH_STATE_H


