

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
#include <stdint.h>
#include <stdlib.h>

#include <debug/assert.h>
#include <player/Linear_controls.h>
#include <player/Slider.h>
#include <player/Work_buffer.h>
#include <Tstamp.h>


void Linear_controls_init(Linear_controls* lc, int32_t audio_rate, double tempo)
{
    assert(lc != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    Linear_controls_set_audio_rate(lc, audio_rate);
    Linear_controls_set_tempo(lc, tempo);

    Linear_controls_reset(lc);

    return;
}


void Linear_controls_set_audio_rate(Linear_controls* lc, int32_t audio_rate)
{
    assert(lc != NULL);
    assert(audio_rate > 0);

    Slider_set_mix_rate(&lc->slider, audio_rate);

    return;
}


void Linear_controls_set_tempo(Linear_controls* lc, double tempo)
{
    assert(lc != NULL);
    assert(tempo > 0);

    Slider_set_tempo(&lc->slider, tempo);

    return;
}


void Linear_controls_reset(Linear_controls* lc)
{
    assert(lc != NULL);

    lc->value = NAN;
    Slider_init(&lc->slider, SLIDE_MODE_LINEAR);

    return;
}


void Linear_controls_set_value(Linear_controls* lc, double value)
{
    assert(lc != NULL);
    assert(isfinite(value));

    lc->value = value;
    Slider_break(&lc->slider);

    return;
}


void Linear_controls_slide_value_target(Linear_controls* lc, double value)
{
    assert(lc != NULL);
    assert(isfinite(value));

    if (Slider_in_progress(&lc->slider))
        Slider_change_target(&lc->slider, value);
    else
        Slider_start(&lc->slider, value, lc->value);

    return;
}


void Linear_controls_slide_value_length(Linear_controls* lc, const Tstamp* length)
{
    assert(lc != NULL);
    assert(length != NULL);

    Slider_set_length(&lc->slider, length);

    return;
}


void Linear_controls_fill_work_buffer(
        Linear_controls* lc,
        const Work_buffer* wb,
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(lc != NULL);
    assert(wb != NULL);
    assert(buf_start < buf_stop);

    float* values = Work_buffer_get_contents_mut(wb);

    if (Slider_in_progress(&lc->slider))
    {
        float new_value = lc->value;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_value = Slider_step(&lc->slider);
            values[i] = new_value;
        }
        lc->value = new_value;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            values[i] = lc->value;
    }

    return;
}


void Linear_controls_copy(
        Linear_controls* restrict dest, const Linear_controls* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(src != dest);

    dest->value = src->value;
    Slider_copy(&dest->slider, &src->slider);

    return;
}


