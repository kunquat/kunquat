

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


#include <player/Linear_controls.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <mathnum/Tstamp.h>
#include <player/Slider.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


void Linear_controls_init(Linear_controls* lc)
{
    rassert(lc != NULL);

    lc->value = NAN;
    lc->min_value = -INFINITY;
    lc->max_value = INFINITY;
    Slider_init(&lc->slider, SLIDE_MODE_LINEAR);
    LFO_init(&lc->lfo, LFO_MODE_LINEAR);

    return;
}


void Linear_controls_set_audio_rate(Linear_controls* lc, int32_t audio_rate)
{
    rassert(lc != NULL);
    rassert(audio_rate > 0);

    Slider_set_audio_rate(&lc->slider, audio_rate);
    LFO_set_audio_rate(&lc->lfo, audio_rate);

    return;
}


void Linear_controls_set_tempo(Linear_controls* lc, double tempo)
{
    rassert(lc != NULL);
    rassert(tempo > 0);

    Slider_set_tempo(&lc->slider, tempo);
    LFO_set_tempo(&lc->lfo, tempo);

    return;
}


void Linear_controls_set_range(Linear_controls* lc, double min_value, double max_value)
{
    rassert(lc != NULL);
    rassert(!isnan(min_value));
    rassert(max_value >= min_value);

    lc->min_value = min_value;
    lc->max_value = max_value;

    return;
}


void Linear_controls_set_value(Linear_controls* lc, double value)
{
    rassert(lc != NULL);
    rassert(isfinite(value));

    lc->value = value;
    Slider_break(&lc->slider);

    return;
}


double Linear_controls_get_value(const Linear_controls* lc)
{
    rassert(lc != NULL);
    return clamp(lc->value, lc->min_value, lc->max_value);
}


void Linear_controls_slide_value_target(Linear_controls* lc, double value)
{
    rassert(lc != NULL);
    rassert(isfinite(value));

    if (Slider_in_progress(&lc->slider))
        Slider_change_target(&lc->slider, value);
    else
        Slider_start(&lc->slider, value, lc->value);

    return;
}


void Linear_controls_slide_value_length(Linear_controls* lc, const Tstamp* length)
{
    rassert(lc != NULL);
    rassert(length != NULL);

    Slider_set_length(&lc->slider, length);

    return;
}


void Linear_controls_osc_speed_value(Linear_controls* lc, double speed)
{
    rassert(lc != NULL);
    rassert(speed >= 0);

    LFO_set_speed(&lc->lfo, speed);

    //if (lc->osc_depth > 0)
    //    LFO_set_depth(&lc->lfo, lc->osc_depth);

    LFO_turn_on(&lc->lfo);

    return;
}


void Linear_controls_osc_depth_value(Linear_controls* lc, double depth)
{
    rassert(lc != NULL);
    rassert(isfinite(depth));

    //if (lc->osc_speed > 0)
    //    LFO_set_speed(&lc->lfo, lc->osc_speed);

    LFO_set_depth(&lc->lfo, depth);
    LFO_turn_on(&lc->lfo);

    return;
}


void Linear_controls_osc_speed_slide_value(Linear_controls* lc, const Tstamp* length)
{
    rassert(lc != NULL);
    rassert(length != NULL);

    LFO_set_speed_slide(&lc->lfo, length);

    return;
}


void Linear_controls_osc_depth_slide_value(Linear_controls* lc, const Tstamp* length)
{
    rassert(lc != NULL);
    rassert(length != NULL);

    LFO_set_depth_slide(&lc->lfo, length);

    return;
}


void Linear_controls_fill_work_buffer(
        Linear_controls* lc,
        Work_buffer* wb,
        int32_t buf_start,
        int32_t buf_stop)
{
    rassert(lc != NULL);
    rassert(wb != NULL);
    rassert(buf_start < buf_stop);

    float* values = Work_buffer_get_contents_mut(wb);

    int32_t const_start = buf_start;

    // Apply slider
    {
        int32_t cur_pos = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps =
                Slider_estimate_active_steps_left(&lc->slider);
            if (estimated_steps > 0)
            {
                int32_t slide_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
                    slide_stop = cur_pos + estimated_steps;

                double new_value = lc->value;
                for (int32_t i = cur_pos; i < slide_stop; ++i)
                {
                    new_value = Slider_step(&lc->slider);
                    values[i] = (float)new_value;
                }
                lc->value = new_value;

                const_start = slide_stop;
                cur_pos = slide_stop;
            }
            else
            {
                const float value = (float)lc->value;
                for (int32_t i = cur_pos; i < buf_stop; ++i)
                    values[i] = value;

                cur_pos = buf_stop;
            }
        }
    }

    // Apply LFO
    {
        int32_t cur_pos = buf_start;
        int32_t final_lfo_stop = buf_start;
        while (cur_pos < buf_stop)
        {
            const int32_t estimated_steps = LFO_estimate_active_steps_left(&lc->lfo);
            if (estimated_steps > 0)
            {
                int32_t lfo_stop = buf_stop;
                if (estimated_steps < buf_stop - cur_pos)
                    lfo_stop = cur_pos + estimated_steps;

                for (int32_t i = cur_pos; i < lfo_stop; ++i)
                    values[i] += (float)LFO_step(&lc->lfo);

                final_lfo_stop = lfo_stop;
                cur_pos = lfo_stop;
            }
            else
            {
                final_lfo_stop = cur_pos;
                break;
            }
        }

        const_start = max(const_start, final_lfo_stop);
    }

    // Clamp values
    if (lc->min_value > -INFINITY)
    {
        const float min_value = (float)lc->min_value;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            values[i] = max(min_value, values[i]);
    }

    if (lc->max_value < INFINITY)
    {
        const float max_value = (float)lc->max_value;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            values[i] = min(max_value, values[i]);
    }

    // Mark constant region of the buffer
    Work_buffer_set_const_start(wb, const_start);

    return;
}


void Linear_controls_skip(Linear_controls* lc, int64_t step_count)
{
    rassert(lc != NULL);
    rassert(step_count >= 0);

    if (Slider_in_progress(&lc->slider))
        lc->value = Slider_skip(&lc->slider, step_count);

    LFO_skip(&lc->lfo, step_count);

    return;
}


void Linear_controls_copy(
        Linear_controls* restrict dest, const Linear_controls* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    dest->value = src->value;
    Slider_copy(&dest->slider, &src->slider);
    LFO_copy(&dest->lfo, &src->lfo);

    return;
}


void Linear_controls_convert(
        Linear_controls* restrict dest,
        double map_min_to,
        double map_max_to,
        const Linear_controls* restrict src,
        double range_min,
        double range_max)
{
    rassert(dest != NULL);
    rassert(isfinite(map_min_to));
    rassert(isfinite(map_max_to));
    rassert(src != NULL);
    rassert(src != dest);
    rassert(isfinite(range_min));
    rassert(isfinite(range_max));

    Linear_controls_copy(dest, src);

    const double value_norm = get_range_norm(src->value, range_min, range_max);
    dest->value = lerp(map_min_to, map_max_to, value_norm);

    Slider_change_range(&dest->slider, range_min, range_max, map_min_to, map_max_to);

    const double src_range_diff = range_max - range_min;
    const double target_range_diff = map_max_to - map_min_to;
    LFO_change_depth_range(&dest->lfo, src_range_diff, target_range_diff);

    // Convert minimum/maximum values
    {
        const bool flip = (range_min <= range_max) != (map_min_to <= map_max_to);

        const double src_range_min_value = min(range_min, range_max);
        const double src_range_max_value = max(range_min, range_max);
        const double src_range_width = src_range_max_value - src_range_min_value;

        double new_min = flip ? src->min_value : src->max_value;
        double new_max = flip ? src->max_value : src->min_value;

        if (isfinite(new_min))
        {
            double norm_unclamped = 0;
            if (src_range_width > 0)
            {
                norm_unclamped = (new_min - src_range_min_value) / src_range_width;
                if (range_min > range_max)
                    norm_unclamped = 1.0 - norm_unclamped;
            }

            new_min = map_min_to + ((map_max_to - map_min_to) * norm_unclamped);
        }
        else if (flip)
        {
            new_min = -new_min;
        }

        if (isfinite(new_max))
        {
            double norm_unclamped = 0;
            if (src_range_width > 0)
            {
                norm_unclamped = (new_max - src_range_min_value) / src_range_width;
                if (range_min > range_max)
                    norm_unclamped = 1.0 - norm_unclamped;
            }

            new_max = map_min_to + ((map_max_to - map_min_to) * norm_unclamped);
        }
        else if (flip)
        {
            new_max = -new_max;
        }

        rassert(new_min <= new_max);

        dest->min_value = new_min;
        dest->max_value = new_max;
    }

    return;
}


