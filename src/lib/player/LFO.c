

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


#include <player/LFO.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <mathnum/fast_sin.h>
#include <player/Player.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void LFO_update_time(LFO* lfo, int32_t audio_rate, double tempo);


LFO* LFO_init(LFO* lfo, LFO_mode mode)
{
    rassert(lfo != NULL);
    rassert(mode == LFO_MODE_LINEAR || mode == LFO_MODE_EXP);

    lfo->mode = mode;
    lfo->audio_rate = DEFAULT_AUDIO_RATE;
    lfo->tempo = DEFAULT_TEMPO;

    lfo->on = false;

    lfo->target_speed = 0;
    lfo->prev_speed = 0;
    Slider_init(&lfo->speed_slider, SLIDE_MODE_LINEAR);
    lfo->target_depth = 0;
    lfo->prev_depth = 0;
    Slider_init(&lfo->depth_slider, SLIDE_MODE_LINEAR);

    lfo->offset = 0;
    lfo->phase = 0;
    lfo->update = 0;

    return lfo;
}


LFO* LFO_copy(LFO* restrict dest, const LFO* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    memcpy(dest, src, sizeof(LFO));
//    Slider_copy(&dest->speed_slider, &src->speed_slider);
//    Slider_copy(&dest->depth_slider, &src->depth_slider);

    return dest;
}


void LFO_set_audio_rate(LFO* lfo, int32_t audio_rate)
{
    rassert(lfo != NULL);
    rassert(audio_rate > 0);

    if (lfo->audio_rate == audio_rate)
        return;

    Slider_set_audio_rate(&lfo->speed_slider, audio_rate);
    Slider_set_audio_rate(&lfo->depth_slider, audio_rate);
    if (!LFO_active(lfo))
    {
        lfo->audio_rate = audio_rate;
        return;
    }

    LFO_update_time(lfo, audio_rate, lfo->tempo);

    return;
}


void LFO_set_tempo(LFO* lfo, double tempo)
{
    rassert(lfo != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    if (lfo->tempo == tempo)
        return;

    Slider_set_tempo(&lfo->speed_slider, tempo);
    Slider_set_tempo(&lfo->depth_slider, tempo);
    if (!LFO_active(lfo))
    {
        lfo->tempo = tempo;
        return;
    }

    LFO_update_time(lfo, lfo->audio_rate, tempo);

    return;
}


void LFO_set_speed(LFO* lfo, double speed)
{
    rassert(lfo != NULL);
    rassert(isfinite(speed));
    rassert(speed >= 0);

    if (Slider_in_progress(&lfo->speed_slider))
    {
        const double cur_progress = Slider_get_value(&lfo->speed_slider);
        lfo->prev_speed = lerp(lfo->prev_speed, lfo->target_speed, cur_progress);
    }
    else
    {
        lfo->prev_speed = lfo->target_speed;
    }

    lfo->target_speed = speed;
    Slider_start(&lfo->speed_slider, 1.0, 0.0);

    return;
}


void LFO_set_speed_slide(LFO* lfo, const Tstamp* length)
{
    rassert(lfo != NULL);
    rassert(length != NULL);
    rassert(Tstamp_cmp(length, Tstamp_init(TSTAMP_AUTO)) >= 0);

    Slider_set_length(&lfo->speed_slider, length);

    return;
}


void LFO_set_depth(LFO* lfo, double depth)
{
    rassert(lfo != NULL);
    rassert(isfinite(depth));

    if (Slider_in_progress(&lfo->depth_slider))
    {
        const double cur_progress = Slider_get_value(&lfo->depth_slider);
        lfo->prev_depth = lerp(lfo->prev_depth, lfo->target_depth, cur_progress);
    }
    else
    {
        lfo->prev_depth = lfo->target_depth;
    }

    lfo->target_depth = depth;
    Slider_start(&lfo->depth_slider, 1.0, 0.0);

    return;
}


void LFO_set_depth_slide(LFO* lfo, const Tstamp* length)
{
    rassert(lfo != NULL);
    rassert(length != NULL);
    rassert(Tstamp_cmp(length, Tstamp_init(TSTAMP_AUTO)) >= 0);

    Slider_set_length(&lfo->depth_slider, length);

    return;
}


void LFO_set_offset(LFO* lfo, double offset)
{
    rassert(lfo != NULL);
    rassert(isfinite(offset));
    rassert(offset >= -1);
    rassert(offset <= 1);

    lfo->offset = offset;

    return;
}


void LFO_turn_on(LFO* lfo)
{
    rassert(lfo != NULL);
    rassert(lfo->audio_rate > 0);
    rassert(lfo->tempo > 0);

    if (!lfo->on)
    {
        lfo->phase = fmod(asin(-lfo->offset), 2 * PI);
        if (lfo->phase < 0)
            lfo->phase += 2 * PI;

        rassert(lfo->phase >= 0);
        rassert(lfo->phase < 2 * PI);
    }

    lfo->on = true;
    return;
}


void LFO_turn_off(LFO* lfo)
{
    rassert(lfo != NULL);
    rassert(lfo->audio_rate > 0);
    rassert(lfo->tempo > 0);

    lfo->on = false;

    return;
}


double LFO_step(LFO* lfo)
{
    rassert(lfo != NULL);
    rassert(lfo->audio_rate > 0);
    rassert(isfinite(lfo->tempo));
    rassert(lfo->tempo > 0);

    if (!LFO_active(lfo))
    {
        if (lfo->mode == LFO_MODE_EXP)
            return 1;

        rassert(lfo->mode == LFO_MODE_LINEAR);
        return 0;
    }

    double cur_speed = lfo->target_speed;

    if (Slider_in_progress(&lfo->speed_slider))
    {
        const double progress = Slider_step(&lfo->speed_slider);
        cur_speed = lerp(lfo->prev_speed, lfo->target_speed, progress);
#if 0
        double unit_len = Tstamp_toframes(
                Tstamp_set(TSTAMP_AUTO, 1, 0),
                lfo->tempo,
                lfo->audio_rate);
        lfo->update = (lfo->speed * (2 * PI)) / unit_len;
#endif
        lfo->update = (cur_speed * (2 * PI)) / lfo->audio_rate;
    }

    double cur_depth = lfo->target_depth;

    if (Slider_in_progress(&lfo->depth_slider))
    {
        const double progress = Slider_step(&lfo->depth_slider);
        cur_depth = lerp(lfo->prev_depth, lfo->target_depth, progress);
    }

    double new_phase = lfo->phase + lfo->update;
    if (new_phase >= (2 * PI))
        new_phase = fmod(new_phase, 2 * PI);

    if ((!Slider_in_progress(&lfo->depth_slider) && cur_depth == 0) ||
            (!lfo->on &&
            (new_phase < lfo->phase ||
             (new_phase >= PI && lfo->phase < PI)))) // TODO: offset
    {
        lfo->phase = 0;
        lfo->update = 0;
        Slider_break(&lfo->speed_slider);
        Slider_break(&lfo->depth_slider);
    }
    else
    {
        lfo->phase = new_phase;
    }

    const double value = fast_sin(lfo->phase) * cur_depth;
    if (lfo->mode == LFO_MODE_EXP)
        return exp2(value);

    rassert(lfo->mode == LFO_MODE_LINEAR);
    return value;
}


double LFO_skip(LFO* lfo, int64_t steps)
{
    rassert(lfo != NULL);
    rassert(steps >= 0);

    if (steps == 0)
    {
        if (lfo->mode == LFO_MODE_EXP)
            return 1;

        rassert(lfo->mode == LFO_MODE_LINEAR);
        return 0;
    }
    else if (steps == 1)
    {
        return LFO_step(lfo);
    }

    const double speed_progress = Slider_skip(&lfo->speed_slider, steps);
    const double cur_speed = lerp(lfo->prev_speed, lfo->target_speed, speed_progress);
    lfo->update = (cur_speed * (2 * PI)) / lfo->audio_rate;

    const double depth_progress = Slider_skip(&lfo->depth_slider, steps);
    const double cur_depth = lerp(lfo->prev_depth, lfo->target_depth, depth_progress);
    lfo->phase = fmod(lfo->phase + (lfo->update * (double)steps), 2 * PI);

    const double value = fast_sin(lfo->phase) * cur_depth;
    if (lfo->mode == LFO_MODE_EXP)
        return exp2(value);

    rassert(lfo->mode == LFO_MODE_LINEAR);
    return value;
}


static void LFO_update_time(LFO* lfo, int32_t audio_rate, double tempo)
{
    rassert(lfo != NULL);
    rassert(audio_rate > 0);
    rassert(tempo > 0);

//    lfo->speed *= (double)audio_rate / lfo->audio_rate;
//    lfo->speed *= lfo->tempo / tempo;
    lfo->update *= (double)lfo->audio_rate / audio_rate;
//    lfo->update *= tempo / lfo->tempo;
    Slider_set_audio_rate(&lfo->speed_slider, audio_rate);
    Slider_set_tempo(&lfo->speed_slider, tempo);
    Slider_set_audio_rate(&lfo->depth_slider, audio_rate);
    Slider_set_tempo(&lfo->depth_slider, tempo);

    lfo->audio_rate = audio_rate;
    lfo->tempo = tempo;

    return;
}


bool LFO_active(const LFO* lfo)
{
    rassert(lfo != NULL);

    return lfo->update > 0 ||
           Slider_in_progress(&lfo->speed_slider) ||
           Slider_in_progress(&lfo->depth_slider);
}


int32_t LFO_estimate_active_steps_left(const LFO* lfo)
{
    rassert(lfo != NULL);

    if (Slider_in_progress(&lfo->depth_slider) && lfo->target_depth == 0)
        return Slider_estimate_active_steps_left(&lfo->depth_slider);

    return LFO_active(lfo) ? INT32_MAX : 0;
}


double LFO_get_target_speed(const LFO* lfo)
{
    rassert(lfo != NULL);
    return lfo->target_speed;
}


double LFO_get_target_depth(const LFO* lfo)
{
    rassert(lfo != NULL);
    return lfo->target_depth;
}


void LFO_change_depth_range(LFO* lfo, double from_depth, double to_depth)
{
    rassert(lfo != NULL);
    rassert(isfinite(from_depth));
    rassert(isfinite(to_depth));

    if (from_depth == 0)
    {
        lfo->target_depth = (lfo->target_depth == 0) ? 0 : to_depth;
        lfo->prev_depth = (lfo->prev_depth == 0) ? 0 : to_depth;
    }
    else
    {
        const double ratio = to_depth / from_depth;
        lfo->target_depth *= ratio;
        lfo->prev_depth *= ratio;
    }

    return;
}


