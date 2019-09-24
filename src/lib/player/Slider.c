

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
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


Slider* Slider_init(Slider* slider)
{
    rassert(slider != NULL);

    slider->audio_rate = DEFAULT_AUDIO_RATE;

    Slider_reset(slider);

    return slider;
}


void Slider_reset(Slider* slider)
{
    rassert(slider != NULL);

    slider->tempo = DEFAULT_TEMPO;

    Tstamp_init(&slider->length);
    slider->from = 0;
    slider->to = 0;

    slider->progress = 1;
    slider->progress_update = 0;

    return;
}


Slider* Slider_copy(Slider* restrict dest, const Slider* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(dest != src);

    memcpy(dest, src, sizeof(Slider));

    return dest;
}


void Slider_start(Slider* slider, double target, double start)
{
    rassert(slider != NULL);
    rassert(isfinite(target));
    rassert(isfinite(start));

    slider->from = start;
    slider->to = target;

    slider->progress = 0;
    slider->progress_update = 1;
    if (Tstamp_cmp(&slider->length, TSTAMP_AUTO) > 0)
        slider->progress_update =
            1.0 / Tstamp_toframes(&slider->length, slider->tempo, slider->audio_rate);

    return;
}


double Slider_get_value(const Slider* slider)
{
    rassert(slider != NULL);

    if (slider->progress >= 1)
        return slider->to;

    return lerp(slider->from, slider->to, slider->progress);
}


double Slider_step(Slider* slider)
{
    rassert(slider != NULL);

    slider->progress += slider->progress_update;

    return Slider_get_value(slider);
}


double Slider_skip(Slider* slider, int64_t steps)
{
    rassert(slider != NULL);
    rassert(steps >= 0);

    slider->progress += slider->progress_update * (double)steps;

    return Slider_get_value(slider);
}


int32_t Slider_estimate_active_steps_left(const Slider* slider)
{
    rassert(slider != NULL);

    if (!Slider_in_progress(slider))
        return 0;

    const double steps = ceil((1 - slider->progress) / slider->progress_update);
    const int32_t steps_i = (steps > INT32_MAX) ? INT32_MAX : (int32_t)steps;

    return max(1, steps_i);
}


void Slider_break(Slider* slider)
{
    rassert(slider != NULL);

    slider->progress = 1;

    return;
}


void Slider_change_target(Slider* slider, double target)
{
    rassert(slider != NULL);
    rassert(isfinite(target));

    if (slider->progress < 1)
        Slider_start(slider, target, Slider_get_value(slider));

    return;
}


void Slider_set_length(Slider* slider, const Tstamp* length)
{
    rassert(slider != NULL);
    rassert(length != NULL);

    Tstamp_copy(&slider->length, length);
    if (slider->progress < 1)
        Slider_start(slider, slider->to, Slider_get_value(slider));

    return;
}


void Slider_set_audio_rate(Slider* slider, int32_t audio_rate)
{
    rassert(slider != NULL);
    rassert(audio_rate > 0);

    if (slider->audio_rate == audio_rate)
        return;

    Slider_update_time(slider, audio_rate, slider->tempo);

    return;
}


void Slider_set_tempo(Slider* slider, double tempo)
{
    rassert(slider != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    if (slider->tempo == tempo)
        return;

    Slider_update_time(slider, slider->audio_rate, tempo);

    return;
}


static void Slider_update_time(Slider* slider, int32_t audio_rate, double tempo)
{
    rassert(slider != NULL);

    slider->audio_rate = audio_rate;
    slider->tempo = tempo;

    if (Tstamp_cmp(&slider->length, TSTAMP_AUTO) > 0)
        slider->progress_update =
            1.0 / Tstamp_toframes(&slider->length, slider->tempo, slider->audio_rate);

    return;
}


bool Slider_in_progress(const Slider* slider)
{
    rassert(slider != NULL);
    return (slider->progress < 1);
}


void Slider_change_range(
        Slider* slider,
        double from_start,
        double from_end,
        double to_start,
        double to_end)
{
    rassert(slider != NULL);
    rassert(isfinite(from_start));
    rassert(isfinite(from_end));
    rassert(isfinite(to_start));
    rassert(isfinite(to_end));

    const double start_norm = get_range_norm(slider->from, from_start, from_end);
    const double target_norm = get_range_norm(slider->to, from_start, from_end);

    slider->from = lerp(to_start, to_end, start_norm);
    slider->to = lerp(to_start, to_end, target_norm);

    return;
}


