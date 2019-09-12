

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
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
    rassert(fc != NULL);

    fc->force = NAN;
    Slider_init(&fc->slider);
    LFO_init(&fc->tremolo, LFO_MODE_LINEAR);

    Force_controls_set_audio_rate(fc, audio_rate);
    Force_controls_set_tempo(fc, tempo);

    return;
}


void Force_controls_set_audio_rate(Force_controls* fc, int32_t audio_rate)
{
    rassert(fc != NULL);
    rassert(audio_rate > 0);

    Slider_set_audio_rate(&fc->slider, audio_rate);
    LFO_set_audio_rate(&fc->tremolo, audio_rate);

    return;
}


void Force_controls_set_tempo(Force_controls* fc, double tempo)
{
    rassert(fc != NULL);
    rassert(tempo > 0);

    Slider_set_tempo(&fc->slider, tempo);
    LFO_set_tempo(&fc->tremolo, tempo);

    return;
}


void Force_controls_reset(Force_controls* fc)
{
    rassert(fc != NULL);

    fc->force = NAN;
    Slider_init(&fc->slider);
    LFO_init(&fc->tremolo, LFO_MODE_LINEAR);

    return;
}


void Force_controls_copy(
        Force_controls* restrict dest, const Force_controls* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    dest->force = src->force;
    Slider_copy(&dest->slider, &src->slider);
    LFO_copy(&dest->tremolo, &src->tremolo);

    return;
}


