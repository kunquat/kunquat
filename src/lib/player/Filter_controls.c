

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


#include <player/Filter_controls.h>

#include <debug/assert.h>
#include <player/LFO.h>
#include <player/Slider.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


void Filter_controls_init(Filter_controls* fc, int32_t audio_rate, double tempo)
{
    assert(fc != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    fc->lowpass = NAN;
    fc->resonance = NAN;

    Filter_controls_set_audio_rate(fc, audio_rate);
    Filter_controls_set_tempo(fc, tempo);

    return;
}


void Filter_controls_set_audio_rate(Filter_controls* fc, int32_t audio_rate)
{
    assert(fc != NULL);
    assert(audio_rate > 0);

    Slider_set_audio_rate(&fc->lowpass_slider, audio_rate);
    LFO_set_audio_rate(&fc->autowah, audio_rate);

    Slider_set_audio_rate(&fc->resonance_slider, audio_rate);

    return;
}


void Filter_controls_set_tempo(Filter_controls* fc, double tempo)
{
    assert(fc != NULL);
    assert(tempo > 0);

    Slider_set_tempo(&fc->lowpass_slider, tempo);
    LFO_set_tempo(&fc->autowah, tempo);

    Slider_set_tempo(&fc->resonance_slider, tempo);

    return;
}


void Filter_controls_reset(Filter_controls* fc)
{
    assert(fc != NULL);

    fc->lowpass = NAN;
    Slider_init(&fc->lowpass_slider, SLIDE_MODE_LINEAR);
    LFO_init(&fc->autowah, LFO_MODE_LINEAR);

    fc->resonance = NAN;
    Slider_init(&fc->resonance_slider, SLIDE_MODE_LINEAR);

    return;
}


void Filter_controls_copy(
        Filter_controls* restrict dest, const Filter_controls* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(src != dest);

    dest->lowpass = src->lowpass;
    Slider_copy(&dest->lowpass_slider, &src->lowpass_slider);
    LFO_copy(&dest->autowah, &src->autowah);

    dest->resonance = src->resonance;
    Slider_copy(&dest->resonance_slider, &src->resonance_slider);

    return;
}


