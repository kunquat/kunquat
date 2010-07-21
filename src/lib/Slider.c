

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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
#include <math.h>

#include <Reltime.h>
#include <Slider.h>
#include <xassert.h>


static void Slider_update_time(Slider* slider,
                               uint32_t mix_rate,
                               double tempo);


Slider* Slider_init(Slider* slider,
                    Slide_mode mode,
                    uint32_t mix_rate,
                    double tempo)
{
    assert(slider != NULL);
    assert(mode == SLIDE_MODE_LINEAR || mode == SLIDE_MODE_EXP);
    assert(mix_rate > 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    slider->mode = mode;
    slider->mix_rate = mix_rate;
    slider->tempo = tempo;

    slider->dir = 0;
    Reltime_init(&slider->length);
    slider->current_value = 0;
    slider->target_value = 0;
    slider->steps_left = 0;
    slider->update = mode == SLIDE_MODE_EXP ? 1 : 0;

    return slider;
}


void Slider_start(Slider* slider,
                  double start,
                  double target,
                  Reltime* length)
{
    assert(slider != NULL);
    assert(isfinite(start));
    assert(isfinite(target));
    assert(length != NULL);

    Reltime_copy(&slider->length, length);
    slider->steps_left = Reltime_toframes(&slider->length,
                                          slider->tempo,
                                          slider->mix_rate);
    slider->current_value = start;
    slider->target_value = target;
    int zero_slide = 0;
    if (slider->mode == SLIDE_MODE_EXP)
    {
        zero_slide = 1;
        slider->update = exp2((log2(target) - log2(start)) /
                              slider->steps_left);
    }
    else
    {
        assert(slider->mode == SLIDE_MODE_LINEAR);
        slider->update = (target - start) / slider->steps_left;
    }
    if (slider->update > zero_slide)
    {
        slider->dir = 1;
    }
    else if (slider->update < zero_slide)
    {
        slider->dir = -1;
    }
    else
    {
        slider->dir = 0;
        slider->current_value = slider->target_value;
        slider->steps_left = 0;
    }
    return;
}


double Slider_step(Slider* slider)
{
    assert(slider != NULL);
    if (slider->dir == 0)
    {
        return slider->target_value;
    }
    if (slider->mode == SLIDE_MODE_EXP)
    {
        slider->current_value *= slider->update;
    }
    else
    {
        assert(slider->mode == SLIDE_MODE_LINEAR);
        slider->current_value += slider->update;
    }
    slider->steps_left -= 1;
    if (slider->steps_left <= 0)
    {
        slider->dir = 0;
        slider->current_value = slider->target_value;
    }
    else if (slider->dir == 1)
    {
        if (slider->current_value > slider->target_value)
        {
            slider->current_value = slider->target_value;
            slider->dir = 0;
        }
    }
    else
    {
        assert(slider->dir == -1);
        if (slider->current_value < slider->target_value)
        {
            slider->current_value = slider->target_value;
            slider->dir = 0;
        }
    }
    return slider->current_value;
}


void Slider_change_target(Slider* slider, double target)
{
    assert(slider != NULL);
    assert(isfinite(target));
    Slider_start(slider, slider->current_value, target, &slider->length);
    return;
}


void Slider_change_length(Slider* slider, Reltime* length)
{
    assert(slider != NULL);
    assert(length != NULL);
    Slider_start(slider, slider->current_value, slider->target_value, length);
    return;
}


void Slider_change_mix_rate(Slider* slider, uint32_t mix_rate)
{
    assert(slider != NULL);
    assert(mix_rate > 0);
    Slider_update_time(slider, mix_rate, slider->tempo);
    return;
}


void Slider_change_tempo(Slider* slider, double tempo)
{
    assert(slider != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);
    Slider_update_time(slider, slider->mix_rate, tempo);
    return;
}


static void Slider_update_time(Slider* slider,
                               uint32_t mix_rate,
                               double tempo)
{
    assert(slider != NULL);
    if (slider->dir == 0)
    {
        return;
    }
    if (slider->mode == SLIDE_MODE_EXP)
    {
        double log_update = log2(slider->update);
        log_update *= (double)slider->mix_rate / mix_rate;
        log_update *= tempo / slider->tempo;
        slider->update = exp2(log_update);
    }
    else
    {
        assert(slider->mode == SLIDE_MODE_LINEAR);
        slider->update *= (double)slider->mix_rate / mix_rate;
        slider->update *= tempo / slider->tempo;
    }
    slider->steps_left *= (double)mix_rate / slider->mix_rate;
    slider->steps_left *= slider->tempo / tempo;

    slider->mix_rate = mix_rate;
    slider->tempo = tempo;
    return;
}


bool Slider_target_reached(Slider* slider)
{
    assert(slider != NULL);
    return slider->steps_left <= 0;
}


