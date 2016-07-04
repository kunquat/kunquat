

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


#include <player/Channel.h>

#include <debug/assert.h>
#include <init/Environment.h>
#include <init/sheet/Channel_defaults.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/Channel_cv_state.h>
#include <player/Channel_stream_state.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool Channel_init(Channel* ch, int num, Env_state* estate, const Module* module)
{
    rassert(ch != NULL);
    rassert(num >= 0);
    rassert(num < KQT_COLUMNS_MAX);
    rassert(estate != NULL);
    rassert(module != NULL);

    General_state_preinit(&ch->parent);

    ch->cvstate = new_Channel_cv_state();
    ch->csstate = new_Channel_stream_state();
    if ((ch->cvstate == NULL) || (ch->csstate == NULL) ||
            !General_state_init(&ch->parent, false, estate, module))
    {
        del_Channel_stream_state(ch->csstate);
        del_Channel_cv_state(ch->cvstate);
        return false;
    }

    char context[] = "chXX";
    snprintf(context, strlen(context) + 1, "ch%02x", num);
    Random_init(&ch->rand, context);

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
    rassert(num >= 0);
    rassert(num < KQT_CHANNELS_MAX);
    rassert(au_table != NULL);
    rassert(estate != NULL);
    rassert(voices != NULL);
    rassert(tempo != NULL);
    rassert(audio_rate != NULL);

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
    rassert(ch != NULL);
    rassert(audio_rate > 0);

    Force_controls_set_audio_rate(&ch->force_controls, audio_rate);
    Pitch_controls_set_audio_rate(&ch->pitch_controls, audio_rate);
    Channel_stream_state_set_audio_rate(ch->csstate, audio_rate);

    return;
}


void Channel_set_tempo(Channel* ch, double tempo)
{
    rassert(ch != NULL);
    rassert(tempo > 0);

    Force_controls_set_tempo(&ch->force_controls, tempo);
    Pitch_controls_set_tempo(&ch->pitch_controls, tempo);
    Channel_stream_state_set_tempo(ch->csstate, tempo);

    return;
}


void Channel_set_random_seed(Channel* ch, uint64_t seed)
{
    rassert(ch != NULL);

    Random_set_seed(&ch->rand, seed);

    return;
}


void Channel_set_event_cache(Channel* ch, Event_cache* cache)
{
    rassert(ch != NULL);
    rassert(cache != NULL);

    del_Event_cache(ch->event_cache);
    ch->event_cache = cache;

    return;
}


void Channel_reset(Channel* ch)
{
    rassert(ch != NULL);

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
    ch->tremolo_speed = 0;
    Tstamp_init(&ch->tremolo_speed_slide);
    ch->tremolo_depth = 0;
    Tstamp_init(&ch->tremolo_depth_slide);
    ch->carry_force = false;
    Force_controls_reset(&ch->force_controls);

    Tstamp_set(&ch->pitch_slide_length, 0, 0);
    ch->vibrato_speed = 0;
    Tstamp_init(&ch->vibrato_speed_slide);
    ch->vibrato_depth = 0;
    Tstamp_init(&ch->vibrato_depth_slide);
    ch->carry_pitch = false;
    ch->orig_pitch = NAN;
    Pitch_controls_reset(&ch->pitch_controls);

    ch->arpeggio_ref = NAN;
    ch->arpeggio_speed = 24;
    ch->arpeggio_edit_pos = 1;
    ch->arpeggio_tones[0] = ch->arpeggio_tones[1] = NAN;

    ch->carry_expression = false;

    Channel_cv_state_reset(ch->cvstate);
    Channel_stream_state_reset(ch->csstate);

    Random_reset(&ch->rand);
    if (ch->event_cache != NULL)
        Event_cache_reset(ch->event_cache);

    return;
}


void Channel_apply_defaults(Channel* ch, const Channel_defaults* ch_defaults)
{
    rassert(ch != NULL);
    rassert(ch_defaults != NULL);

    ch->au_input = ch_defaults->control_num;
    bool success = Active_names_set(
            ch->parent.active_names, ACTIVE_CAT_INIT_EXPRESSION, ch_defaults->init_expr);
    rassert(success);

    return;
}


Random* Channel_get_random_source(Channel* ch)
{
    rassert(ch != NULL);
    return &ch->rand;
}


Voice* Channel_get_fg_voice(Channel* ch, int proc_index)
{
    rassert(ch != NULL);
    rassert(proc_index >= 0);
    rassert(proc_index < KQT_PROCESSORS_MAX);

    return ch->fg[proc_index];
}


double Channel_get_fg_force(const Channel* ch)
{
    rassert(ch != NULL);

    bool has_fg_voice = false;
    for (int i = 0; i < KQT_PROCESSORS_MAX; ++i)
    {
        if (ch->fg[i] != NULL)
        {
            has_fg_voice = true;
            break;
        }
    }
    if (!has_fg_voice)
        return NAN;

    return ch->force_controls.force;
}


const Channel_cv_state* Channel_get_cv_state(const Channel* ch)
{
    rassert(ch != NULL);
    return ch->cvstate;
}


Channel_cv_state* Channel_get_cv_state_mut(Channel* ch)
{
    rassert(ch != NULL);
    return ch->cvstate;
}


const Channel_stream_state* Channel_get_stream_state(const Channel* ch)
{
    rassert(ch != NULL);
    return ch->csstate;
}


Channel_stream_state* Channel_get_stream_state_mut(Channel* ch)
{
    rassert(ch != NULL);
    return ch->csstate;
}


void Channel_deinit(Channel* ch)
{
    if (ch == NULL)
        return;

    del_Event_cache(ch->event_cache);
    ch->event_cache = NULL;
    del_Channel_stream_state(ch->csstate);
    ch->csstate = NULL;
    del_Channel_cv_state(ch->cvstate);
    ch->cvstate = NULL;
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


