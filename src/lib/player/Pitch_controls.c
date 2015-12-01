

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

#include <debug/assert.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Slider.h>


void Pitch_controls_init(Pitch_controls* pc, int32_t audio_rate, double tempo)
{
    assert(pc != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    pc->pitch = NAN;
    pc->orig_carried_pitch = NAN;
    pc->freq_mul = 1;
    Pitch_controls_set_audio_rate(pc, audio_rate);
    Pitch_controls_set_tempo(pc, tempo);

    return;
}


void Pitch_controls_set_audio_rate(Pitch_controls* pc, int32_t audio_rate)
{
    assert(pc != NULL);
    assert(audio_rate > 0);

    Slider_set_audio_rate(&pc->slider, audio_rate);
    LFO_set_mix_rate(&pc->vibrato, audio_rate);

    return;
}


void Pitch_controls_set_tempo(Pitch_controls* pc, double tempo)
{
    assert(pc != NULL);
    assert(tempo > 0);

    Slider_set_tempo(&pc->slider, tempo);
    LFO_set_tempo(&pc->vibrato, tempo);

    return;
}


void Pitch_controls_reset(Pitch_controls* pc)
{
    assert(pc != NULL);

    pc->pitch = NAN;
    pc->orig_carried_pitch = NAN;
    pc->freq_mul = 1;
    Slider_init(&pc->slider, SLIDE_MODE_EXP);
    LFO_init(&pc->vibrato, LFO_MODE_EXP);

    return;
}


void Pitch_controls_copy(
        Pitch_controls* restrict dest, const Pitch_controls* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(src != dest);

    dest->pitch = src->pitch;
    dest->orig_carried_pitch = src->orig_carried_pitch;
    dest->freq_mul = src->freq_mul;
    Slider_copy(&dest->slider, &src->slider);
    LFO_copy(&dest->vibrato, &src->vibrato);

    return;
}


