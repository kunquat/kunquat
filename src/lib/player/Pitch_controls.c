

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Pitch_controls.h>

#include <debug/assert.h>
#include <player/LFO.h>
#include <player/Slider.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


void Pitch_controls_init(Pitch_controls* pc, int32_t audio_rate, double tempo)
{
    rassert(pc != NULL);
    rassert(audio_rate > 0);
    rassert(tempo > 0);

    pc->pitch = NAN;
    pc->orig_carried_pitch = NAN;
    pc->pitch_add = 0;
    Slider_init(&pc->slider, SLIDE_MODE_LINEAR);
    LFO_init(&pc->vibrato, LFO_MODE_LINEAR);
    Pitch_controls_set_audio_rate(pc, audio_rate);
    Pitch_controls_set_tempo(pc, tempo);

    return;
}


void Pitch_controls_set_audio_rate(Pitch_controls* pc, int32_t audio_rate)
{
    rassert(pc != NULL);
    rassert(audio_rate > 0);

    Slider_set_audio_rate(&pc->slider, audio_rate);
    LFO_set_audio_rate(&pc->vibrato, audio_rate);

    return;
}


void Pitch_controls_set_tempo(Pitch_controls* pc, double tempo)
{
    rassert(pc != NULL);
    rassert(tempo > 0);

    Slider_set_tempo(&pc->slider, tempo);
    LFO_set_tempo(&pc->vibrato, tempo);

    return;
}


void Pitch_controls_reset(Pitch_controls* pc)
{
    rassert(pc != NULL);

    pc->pitch = NAN;
    pc->orig_carried_pitch = NAN;
    pc->pitch_add = 0;
    Slider_init(&pc->slider, SLIDE_MODE_LINEAR);
    LFO_init(&pc->vibrato, LFO_MODE_LINEAR);

    return;
}


void Pitch_controls_copy(
        Pitch_controls* restrict dest, const Pitch_controls* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    dest->pitch = src->pitch;
    dest->orig_carried_pitch = src->orig_carried_pitch;
    dest->pitch_add = src->pitch_add;
    Slider_copy(&dest->slider, &src->slider);
    LFO_copy(&dest->vibrato, &src->vibrato);

    return;
}


