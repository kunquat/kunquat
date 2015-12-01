

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <mathnum/fast_sin.h>
#include <player/LFO.h>
#include <player/Player.h>


static void LFO_update_time(LFO* lfo, int32_t audio_rate, double tempo);


LFO* LFO_init(LFO* lfo, LFO_mode mode)
{
    assert(lfo != NULL);
    assert(mode == LFO_MODE_LINEAR || mode == LFO_MODE_EXP);

    lfo->mode = mode;
    lfo->audio_rate = DEFAULT_AUDIO_RATE;
    lfo->tempo = DEFAULT_TEMPO;

    lfo->on = false;

    lfo->speed = 0;
    Slider_init(&lfo->speed_slider, SLIDE_MODE_LINEAR);
    lfo->depth = 0;
    Slider_init(&lfo->depth_slider, SLIDE_MODE_LINEAR);

    lfo->offset = 0;
    lfo->phase = 0;
    lfo->update = 0;

    return lfo;
}


LFO* LFO_copy(LFO* restrict dest, const LFO* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(src != dest);

    memcpy(dest, src, sizeof(LFO));
//    Slider_copy(&dest->speed_slider, &src->speed_slider);
//    Slider_copy(&dest->depth_slider, &src->depth_slider);

    return dest;
}


void LFO_set_audio_rate(LFO* lfo, int32_t audio_rate)
{
    assert(lfo != NULL);
    assert(audio_rate > 0);

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
    assert(lfo != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

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
    assert(lfo != NULL);
    assert(isfinite(speed));
    assert(speed >= 0);

    if (Slider_in_progress(&lfo->speed_slider))
        Slider_change_target(&lfo->speed_slider, speed);
    else
        Slider_start(&lfo->speed_slider, speed, lfo->speed);

    return;
}


void LFO_set_speed_slide(LFO* lfo, const Tstamp* length)
{
    assert(lfo != NULL);
    assert(length != NULL);
    assert(Tstamp_cmp(length, Tstamp_init(TSTAMP_AUTO)) >= 0);

    Slider_set_length(&lfo->speed_slider, length);

    return;
}


void LFO_set_depth(LFO* lfo, double depth)
{
    assert(lfo != NULL);
    assert(isfinite(depth));

    if (Slider_in_progress(&lfo->depth_slider))
        Slider_change_target(&lfo->depth_slider, depth);
    else
        Slider_start(&lfo->depth_slider, depth, lfo->depth);

    return;
}


void LFO_set_depth_slide(LFO* lfo, const Tstamp* length)
{
    assert(lfo != NULL);
    assert(length != NULL);
    assert(Tstamp_cmp(length, Tstamp_init(TSTAMP_AUTO)) >= 0);

    Slider_set_length(&lfo->depth_slider, length);

    return;
}


void LFO_set_offset(LFO* lfo, double offset)
{
    assert(lfo != NULL);
    assert(isfinite(offset));
    assert(offset >= -1);
    assert(offset <= 1);

    lfo->offset = offset;

    return;
}


void LFO_turn_on(LFO* lfo)
{
    assert(lfo != NULL);
    assert(lfo->audio_rate > 0);
    assert(lfo->tempo > 0);

    if (!lfo->on)
    {
        lfo->phase = fmod(asin(-lfo->offset), 2 * PI);
        if (lfo->phase < 0)
            lfo->phase += 2 * PI;

        assert(lfo->phase >= 0);
        assert(lfo->phase < 2 * PI);
    }

    lfo->on = true;
    return;
}


void LFO_turn_off(LFO* lfo)
{
    assert(lfo != NULL);
    assert(lfo->audio_rate > 0);
    assert(lfo->tempo > 0);

    lfo->on = false;

    return;
}


double LFO_step(LFO* lfo)
{
    assert(lfo != NULL);
    assert(lfo->audio_rate > 0);
    assert(isfinite(lfo->tempo));
    assert(lfo->tempo > 0);

    if (!LFO_active(lfo))
    {
        if (lfo->mode == LFO_MODE_EXP)
            return 1;

        assert(lfo->mode == LFO_MODE_LINEAR);
        return 0;
    }

    if (Slider_in_progress(&lfo->speed_slider))
    {
        lfo->speed = Slider_step(&lfo->speed_slider);
#if 0
        double unit_len = Tstamp_toframes(
                Tstamp_set(TSTAMP_AUTO, 1, 0),
                lfo->tempo,
                lfo->audio_rate);
        lfo->update = (lfo->speed * (2 * PI)) / unit_len;
#endif
        lfo->update = (lfo->speed * (2 * PI)) / lfo->audio_rate;
    }

    if (Slider_in_progress(&lfo->depth_slider))
        lfo->depth = Slider_step(&lfo->depth_slider);

    double new_phase = lfo->phase + lfo->update;
    if (new_phase >= (2 * PI))
        new_phase = fmod(new_phase, 2 * PI);

    if (!lfo->on && (new_phase < lfo->phase ||
                (new_phase >= PI && lfo->phase < PI))) // TODO: offset
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

    const double value = fast_sin(lfo->phase) * lfo->depth;
    if (lfo->mode == LFO_MODE_EXP)
        return exp2(value);

    assert(lfo->mode == LFO_MODE_LINEAR);
    return value;
}


double LFO_skip(LFO* lfo, uint64_t steps)
{
    assert(lfo != NULL);

    if (steps == 0)
    {
        if (lfo->mode == LFO_MODE_EXP)
            return 1;

        assert(lfo->mode == LFO_MODE_LINEAR);
        return 0;
    }
    else if (steps == 1)
    {
        return LFO_step(lfo);
    }

    lfo->speed = Slider_skip(&lfo->speed_slider, steps);
    lfo->update = (lfo->speed * (2 * PI)) / lfo->audio_rate;
    lfo->depth = Slider_skip(&lfo->depth_slider, steps);
    lfo->phase = fmod(lfo->phase + (lfo->update * steps), 2 * PI);

    const double value = fast_sin(lfo->phase) * lfo->depth;
    if (lfo->mode == LFO_MODE_EXP)
        return exp2(value);

    assert(lfo->mode == LFO_MODE_LINEAR);
    return value;
}


static void LFO_update_time(LFO* lfo, int32_t audio_rate, double tempo)
{
    assert(lfo != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);

    lfo->speed *= (double)audio_rate / lfo->audio_rate;
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
    assert(lfo != NULL);

    return lfo->update > 0 ||
           Slider_in_progress(&lfo->speed_slider) ||
           Slider_in_progress(&lfo->depth_slider);
}


