

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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <debug/assert.h>
#include <memory.h>
#include <module/Environment.h>
#include <module/sheet/Channel_defaults.h>
#include <player/Channel.h>
#include <Tstamp.h>


static bool Channel_init(
        Channel* ch,
        int num,
        Env_state* estate,
        const Module* module)
{
    assert(ch != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(estate != NULL);
    assert(module != NULL);

    General_state_preinit(&ch->parent);

    ch->cpstate = new_Channel_proc_state();
    ch->rand = new_Random();
    if (ch->cpstate == NULL || ch->rand == NULL ||
            !General_state_init(&ch->parent, false, estate, module))
    {
        del_Channel_proc_state(ch->cpstate);
        del_Random(ch->rand);
        return false;
    }

    char context[] = "chXX";
    snprintf(context, strlen(context) + 1, "ch%02x", num);
    Random_set_context(ch->rand, context);
    ch->event_cache = NULL;
    ch->num = num;

    Channel_reset(ch);

    return true;
}


Channel* new_Channel(
        const Module* module,
        int num,
        Au_table* au_table,
        Env_state* estate,
        Voice_pool* voices,
        double* tempo,
        int32_t* audio_rate)
{
    assert(num >= 0);
    assert(num < KQT_CHANNELS_MAX);
    assert(au_table != NULL);
    assert(estate != NULL);
    assert(voices != NULL);
    assert(tempo != NULL);
    assert(audio_rate != NULL);

    Channel* ch = memory_alloc_item(Channel);
    if (ch == NULL)
        return NULL;

    if (!Channel_init(ch, num, estate, module))
    {
        memory_free(ch);
        return NULL;
    }

    ch->au_table = au_table;
    ch->pool = voices;
    ch->tempo = tempo;
    ch->freq = audio_rate;

    return ch;
}


void Channel_set_audio_rate(Channel* ch, int32_t audio_rate)
{
    assert(ch != NULL);
    assert(audio_rate > 0);

    Force_controls_set_audio_rate(&ch->force_controls, audio_rate);
    Pitch_controls_set_audio_rate(&ch->pitch_controls, audio_rate);
    Slider_set_mix_rate(&ch->panning_slider, audio_rate);
    LFO_set_mix_rate(&ch->autowah, audio_rate);

    return;
}


void Channel_set_tempo(Channel* ch, double tempo)
{
    assert(ch != NULL);
    assert(tempo > 0);

    Force_controls_set_tempo(&ch->force_controls, tempo);
    Pitch_controls_set_tempo(&ch->pitch_controls, tempo);
    Slider_set_tempo(&ch->panning_slider, tempo);
    LFO_set_tempo(&ch->autowah, tempo);

    return;
}


void Channel_set_random_seed(Channel* ch, uint64_t seed)
{
    assert(ch != NULL);

    Random_set_seed(ch->rand, seed);

    return;
}


void Channel_set_event_cache(Channel* ch, Event_cache* cache)
{
    assert(ch != NULL);
    assert(cache != NULL);

    del_Event_cache(ch->event_cache);
    ch->event_cache = cache;

    return;
}


void Channel_reset(Channel* ch)
{
    assert(ch != NULL);

    General_state_reset(&ch->parent);

    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        ch->fg[i] = NULL;
        ch->fg_id[i] = 0;
    }
    ch->fg_count = 0;

    ch->au_input = 0;

    ch->volume = 1;

    Tstamp_set(&ch->force_slide_length, 0, 0);
    //LFO_init(&ch->tremolo, LFO_MODE_EXP);
    ch->tremolo_speed = 0;
    Tstamp_init(&ch->tremolo_speed_slide);
    ch->tremolo_depth = 0;
    Tstamp_init(&ch->tremolo_depth_slide);
    ch->carry_force = false;
    Force_controls_reset(&ch->force_controls);

    Tstamp_set(&ch->pitch_slide_length, 0, 0);
    //LFO_init(&ch->vibrato, LFO_MODE_EXP);
    ch->vibrato_speed = 0;
    Tstamp_init(&ch->vibrato_speed_slide);
    ch->vibrato_depth = 0;
    Tstamp_init(&ch->vibrato_depth_slide);
    ch->carry_pitch = false;
    ch->orig_pitch = NAN;
    Pitch_controls_reset(&ch->pitch_controls);

    Tstamp_set(&ch->filter_slide_length, 0, 0);
    LFO_init(&ch->autowah, LFO_MODE_EXP);
    ch->autowah_speed = 0;
    Tstamp_init(&ch->autowah_speed_slide);
    ch->autowah_depth = 0;
    Tstamp_init(&ch->autowah_depth_slide);
    Tstamp_set(&ch->lowpass_resonance_slide_length, 0, 0);

    ch->panning = 0;
    Slider_init(&ch->panning_slider, SLIDE_MODE_LINEAR);

    ch->arpeggio_ref = NAN;
    ch->arpeggio_speed = 24;
    ch->arpeggio_edit_pos = 1;
    ch->arpeggio_tones[0] = ch->arpeggio_tones[1] = NAN;

    Random_reset(ch->rand);
    if (ch->event_cache != NULL)
        Event_cache_reset(ch->event_cache);

    return;
}


void Channel_apply_defaults(Channel* ch, const Channel_defaults* ch_defaults)
{
    assert(ch != NULL);
    assert(ch_defaults != NULL);

    ch->au_input = ch_defaults->control_num;

    return;
}


double Channel_get_fg_force(Channel* ch, int proc_index)
{
    assert(ch != NULL);
    assert(proc_index >= 0);
    assert(proc_index < KQT_PROCESSORS_MAX);

    if (ch->fg[proc_index] == NULL)
        return NAN;

    return Voice_get_actual_force(ch->fg[proc_index]);
}


void Channel_deinit(Channel* ch)
{
    if (ch == NULL)
        return;

    del_Event_cache(ch->event_cache);
    ch->event_cache = NULL;
    del_Channel_proc_state(ch->cpstate);
    ch->cpstate = NULL;
    del_Random(ch->rand);
    ch->rand = NULL;
    General_state_deinit(&ch->parent);

    return;
}


void del_Channel(Channel* ch)
{
    if (ch == NULL)
        return;

    Channel_deinit(ch);
    memory_free(ch);

    return;
}


