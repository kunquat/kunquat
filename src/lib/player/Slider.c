

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Slider.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <player/Player.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


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


double Slider_get_value(const Slider* slider)
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


double Slider_skip(Slider* slider, int64_t steps)
{
    assert(slider != NULL);
    assert(steps >= 0);

    slider->progress += slider->progress_update * (double)steps;

    return Slider_get_value(slider);
}


int32_t Slider_estimate_active_steps_left(const Slider* slider)
{
    assert(slider != NULL);

    if (!Slider_in_progress(slider))
        return 0;

    const double steps = ceil((1 - slider->progress) / slider->progress_update);
    const int32_t steps_i = (steps > INT32_MAX) ? INT32_MAX : (int32_t)steps;

    return max(1, steps_i);
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


void Slider_change_range(
        Slider* slider,
        double from_start,
        double from_end,
        double to_start,
        double to_end)
{
    assert(slider != NULL);
    assert(isfinite(from_start));
    assert(isfinite(from_end));
    assert(isfinite(to_start));
    assert(isfinite(to_end));

    const double start_norm = get_range_norm(slider->from, from_start, from_end);
    const double target_norm = get_range_norm(slider->to, from_start, from_end);

    slider->from = lerp(to_start, to_end, start_norm);
    slider->to = lerp(to_start, to_end, target_norm);

    if (slider->mode == SLIDE_MODE_EXP)
    {
        slider->log2_from = log2(slider->from);
        slider->log2_to = log2(slider->to);
    }

    return;
}


