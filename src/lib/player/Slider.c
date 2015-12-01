

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


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <player/Player.h>
#include <player/Slider.h>


static void Slider_update_time(Slider* slider, int32_t audio_rate, double tempo);


Slider* Slider_init(Slider* slider, Slide_mode mode)
{
    assert(slider != NULL);
    assert(mode == SLIDE_MODE_LINEAR || mode == SLIDE_MODE_EXP);

    slider->mode = mode;
    slider->audio_rate = DEFAULT_AUDIO_RATE;
    slider->tempo = DEFAULT_TEMPO;

    Tstamp_init(&slider->length);
    slider->from = 0;
    slider->to = 0;

    slider->progress = 1;
    slider->progress_update = 0;
    slider->log2_from = 0;
    slider->log2_to = 0;

    return slider;
}


Slider* Slider_copy(Slider* restrict dest, const Slider* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);

    memcpy(dest, src, sizeof(Slider));

    return dest;
}


void Slider_start(Slider* slider, double target, double start)
{
    assert(slider != NULL);
    assert(isfinite(target));
    assert(isfinite(start));

    slider->from = start;
    slider->to = target;

    if (slider->mode == SLIDE_MODE_EXP)
    {
        slider->log2_from = log2(slider->from);
        slider->log2_to = log2(slider->to);
    }

    slider->progress = 0;
    slider->progress_update = 1;
    if (Tstamp_cmp(&slider->length, TSTAMP_AUTO) > 0)
        slider->progress_update =
            1.0 / Tstamp_toframes(&slider->length, slider->tempo, slider->audio_rate);

    return;
}


static double Slider_get_value(const Slider* slider)
{
    assert(slider != NULL);

    if (slider->progress >= 1)
        return slider->to;

    if (slider->mode == SLIDE_MODE_EXP)
        return exp2(lerp(slider->log2_from, slider->log2_to, slider->progress));

    return lerp(slider->from, slider->to, slider->progress);
}


double Slider_step(Slider* slider)
{
    assert(slider != NULL);

    slider->progress += slider->progress_update;

    return Slider_get_value(slider);
}


double Slider_skip(Slider* slider, uint64_t steps)
{
    assert(slider != NULL);

    slider->progress += slider->progress_update * steps;

    return Slider_get_value(slider);
}


void Slider_break(Slider* slider)
{
    assert(slider != NULL);

    slider->progress = 1;

    return;
}


void Slider_change_target(Slider* slider, double target)
{
    assert(slider != NULL);
    assert(isfinite(target));

    slider->to = target;
    if (slider->progress < 1)
        Slider_start(slider, target, Slider_get_value(slider));

    return;
}


void Slider_set_length(Slider* slider, const Tstamp* length)
{
    assert(slider != NULL);
    assert(length != NULL);

    Tstamp_copy(&slider->length, length);
    if (slider->progress < 1)
        Slider_start(slider, slider->to, Slider_get_value(slider));

    return;
}


void Slider_set_audio_rate(Slider* slider, int32_t audio_rate)
{
    assert(slider != NULL);
    assert(audio_rate > 0);

    if (slider->audio_rate == audio_rate)
        return;

    Slider_update_time(slider, audio_rate, slider->tempo);

    return;
}


void Slider_set_tempo(Slider* slider, double tempo)
{
    assert(slider != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (slider->tempo == tempo)
        return;

    Slider_update_time(slider, slider->audio_rate, tempo);

    return;
}


static void Slider_update_time(Slider* slider, int32_t audio_rate, double tempo)
{
    assert(slider != NULL);

    slider->audio_rate = audio_rate;
    slider->tempo = tempo;

    if (Tstamp_cmp(&slider->length, TSTAMP_AUTO) > 0)
        slider->progress_update =
            1.0 / Tstamp_toframes(&slider->length, slider->tempo, slider->audio_rate);

    return;
}


bool Slider_in_progress(const Slider* slider)
{
    assert(slider != NULL);
    return (slider->progress < 1);
}


