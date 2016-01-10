

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


#include <player/Force_controls.h>

#include <debug/assert.h>
#include <player/LFO.h>
#include <player/Slider.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


void Force_controls_init(Force_controls* fc, int32_t audio_rate, double tempo)
{
    assert(fc != NULL);

    fc->force = NAN;
    Slider_init(&fc->slider, SLIDE_MODE_EXP);
    LFO_init(&fc->tremolo, LFO_MODE_EXP);

    Force_controls_set_audio_rate(fc, audio_rate);
    Force_controls_set_tempo(fc, tempo);

    return;
}


void Force_controls_set_audio_rate(Force_controls* fc, int32_t audio_rate)
{
    assert(fc != NULL);
    assert(audio_rate > 0);

    Slider_set_audio_rate(&fc->slider, audio_rate);
    LFO_set_audio_rate(&fc->tremolo, audio_rate);

    return;
}


void Force_controls_set_tempo(Force_controls* fc, double tempo)
{
    assert(fc != NULL);
    assert(tempo > 0);

    Slider_set_tempo(&fc->slider, tempo);
    LFO_set_tempo(&fc->tremolo, tempo);

    return;
}


void Force_controls_reset(Force_controls* fc)
{
    assert(fc != NULL);

    fc->force = NAN;
    Slider_init(&fc->slider, SLIDE_MODE_EXP);
    LFO_init(&fc->tremolo, LFO_MODE_EXP);

    return;
}


void Force_controls_copy(
        Force_controls* restrict dest, const Force_controls* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(src != dest);

    dest->force = src->force;
    Slider_copy(&dest->slider, &src->slider);
    LFO_copy(&dest->tremolo, &src->tremolo);

    return;
}


