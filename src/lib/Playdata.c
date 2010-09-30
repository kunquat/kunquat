

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Voice_pool.h>
#include <Channel.h>
#include <Random.h>
#include <Reltime.h>
#include <Slider.h>
#include <Playdata.h>
#include <xassert.h>
#include <xmemory.h>


Playdata* new_Playdata(Ins_table* insts,
                       Random* random)
{
    assert(insts != NULL);
    (void)insts; // FIXME: remove?
    assert(random != NULL);
    Playdata* play = xalloc(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    General_state_init(&play->parent);
    play->random = random;
    play->play_id = 1;
    play->silent = false;
    play->citer = new_Column_iter(NULL);
    if (play->citer == NULL)
    {
        xfree(play);
        return NULL;
    }
    play->voice_pool = new_Voice_pool(256, 64);
    if (play->voice_pool == NULL)
    {
        del_Column_iter(play->citer);
        xfree(play);
        return NULL;
    }
    play->mode = PLAY_SONG;
    play->freq = 48000;
    play->old_freq = play->freq;
    play->subsongs = NULL;
//    play->events = NULL;

    play->scales = NULL;
    play->active_scale = NULL;
    play->scale = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Reltime_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Reltime_init(&play->jump_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
    Slider_set_mix_rate(&play->volume_slider, 48000);

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    Reltime_init(&play->tempo_slide_length);
    play->tempo_slide_target = 0;
    Reltime_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Reltime_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Reltime_init(&play->delay_left);
    play->delay_event_index = -1;

    play->subsong = 0;
    play->section = 0;
    play->pattern = 0;
    Reltime_init(&play->play_time);
    play->play_frames = 0;
    Reltime_init(&play->pos);
    Playdata_reset_stats(play);

    return play;
}


Playdata* new_Playdata_silent(uint32_t freq)
{
    assert(freq > 0);
    Playdata* play = xalloc(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    General_state_init(&play->parent);
    play->random = NULL;
    play->play_id = 0x8000000000000001ULL; // prevent conflict with normal state
    play->silent = true;
    play->citer = new_Column_iter(NULL);
    if (play->citer == NULL)
    {
        xfree(play);
        return NULL;
    }
//    play->ins_events = NULL;
    play->voice_pool = NULL;
    play->mode = PLAY_SONG;
    play->freq = freq;
    play->old_freq = play->freq;
    play->subsongs = NULL;
//    play->events = NULL;

    play->scales = NULL;
    play->active_scale = NULL;
    play->scale = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Reltime_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Reltime_init(&play->jump_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
    Slider_set_mix_rate(&play->volume_slider, freq);

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    Reltime_init(&play->tempo_slide_length);
    play->tempo_slide_target = 0;
    Reltime_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Reltime_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Reltime_init(&play->delay_left);
    play->delay_event_index = -1;

    play->subsong = 0;
    play->section = 0;
    play->pattern = 0;
    Reltime_init(&play->play_time);
    play->play_frames = 0;
    Reltime_init(&play->pos);
    Playdata_reset_stats(play);

    return play;
}


void Playdata_set_mix_freq(Playdata* play, uint32_t freq)
{
    assert(play != NULL);
    assert(freq > 0);
    play->freq = freq;
    Slider_set_mix_rate(&play->volume_slider, freq);
    return;
}


void Playdata_set_subsong(Playdata* play, int subsong)
{
    assert(play != NULL);
    assert(subsong >= 0);
    assert(subsong < KQT_SUBSONGS_MAX);
    play->subsong = subsong;
    play->section = 0;
    if (!play->silent)
    {
        assert(play->voice_pool != NULL);
        Voice_pool_reset(play->voice_pool);
    }
    Subsong* ss = Subsong_table_get(play->subsongs, subsong);
    if (ss == NULL)
    {
        play->tempo = 120;
        return;
    }
    play->tempo = Subsong_get_tempo(ss);
    return;
}


void Playdata_reset(Playdata* play)
{
    assert(play != NULL);
    General_state_init(&play->parent);
    ++play->play_id;
    if (!play->silent)
    {
        Voice_pool_reset(play->voice_pool);
        for (int i = 0; i < KQT_SCALES_MAX; ++i)
        {
            if (play->scales[i] != NULL)
            {
                Scale_retune(play->scales[i], -1, 0);
            }
        }
    }
    if (play->random != NULL)
    {
        Random_reset(play->random);
    }
    play->scale = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Reltime_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Reltime_init(&play->jump_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
//    Slider_set_mix_rate(&play->volume_slider, play->freq);
//    Slider_set_tempo(&play->volume_slider, play->tempo);
    play->tempo_slide = 0;
    Reltime_init(&play->tempo_slide_length);
    Reltime_init(&play->delay_left);
    play->play_frames = 0;
    play->section = 0;
    play->pattern = 0;
    Reltime_init(&play->play_time);
    Reltime_init(&play->pos);
    return;
}


void Playdata_reset_stats(Playdata* play)
{
    assert(play != NULL);
    play->active_voices = 0;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        play->min_amps[i] = INFINITY;
        play->max_amps[i] = -INFINITY;
        play->clipped[i] = 0;
    }
    return;
}


void del_Playdata(Playdata* play)
{
    if (play == NULL)
    {
        return;
    }
    del_Voice_pool(play->voice_pool);
    del_Column_iter(play->citer);
    xfree(play);
    return;
}


