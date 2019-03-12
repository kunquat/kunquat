

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


#include <player/Channel.h>

#include <debug/assert.h>
#include <init/Environment.h>
#include <init/sheet/Channel_defaults.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/Channel_stream_state.h>
#include <player/Voice_group.h>

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

    ch->csstate = new_Channel_stream_state();
    if ((ch->csstate == NULL) || !General_state_init(&ch->parent, false, estate, module))
    {
        del_Channel_stream_state(ch->csstate);
        return false;
    }

    {
        char context[] = "chXX";
        snprintf(context, strlen(context) + 1, "ch%02x", num);
        Random_init(&ch->rand, context);
    }

    {
        char context[] = "chexprXX";
        snprintf(context, strlen(context) + 1, "chexpr%02x", num);
        Random_init(&ch->expr_rand, context);
    }

    ch->event_cache = NULL;
    ch->num = num;
    ch->fg_group_temp = *VOICE_GROUP_AUTO;
    ch->frame_offset_temp = 0;
    ch->mute = false;

    Channel_reset(ch);

    return true;
}


Channel* new_Channel(
        const Module* module,
        int num,
        Au_table* au_table,
        Env_state* estate,
        Voice_pool* voices,
        Voice_group_reservations* voice_group_res,
        double tempo,
        int32_t audio_rate)
{
    rassert(num >= 0);
    rassert(num < KQT_CHANNELS_MAX);
    rassert(au_table != NULL);
    rassert(estate != NULL);
    rassert(voices != NULL);
    rassert(voice_group_res != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);
    rassert(audio_rate > 0);

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
    ch->voice_group_res = voice_group_res;
    ch->tempo = tempo;
    ch->audio_rate = audio_rate;

    return ch;
}


void Channel_set_audio_rate(Channel* ch, int32_t audio_rate)
{
    rassert(ch != NULL);
    rassert(audio_rate > 0);

    ch->audio_rate = audio_rate;

    Force_controls_set_audio_rate(&ch->force_controls, audio_rate);
    Pitch_controls_set_audio_rate(&ch->pitch_controls, audio_rate);
    Channel_stream_state_set_audio_rate(ch->csstate, audio_rate);

    return;
}


void Channel_set_tempo(Channel* ch, double tempo)
{
    rassert(ch != NULL);
    rassert(tempo > 0);

    ch->tempo = tempo;

    Force_controls_set_tempo(&ch->force_controls, tempo);
    Pitch_controls_set_tempo(&ch->pitch_controls, tempo);
    Channel_stream_state_set_tempo(ch->csstate, tempo);

    return;
}


void Channel_set_random_seed(Channel* ch, uint64_t seed)
{
    rassert(ch != NULL);

    Random_set_seed(&ch->rand, seed);
    Random_set_seed(&ch->expr_rand, seed);

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

    Channel_event_buffer_init(&ch->local_events);

    ch->fg_group_id = 0;

    ch->use_test_output = false;
    ch->test_proc_index = -1;
    memset(ch->test_proc_param, '\0', KQT_VAR_NAME_MAX + 1);

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

    memset(ch->init_ch_expression, '\0', KQT_VAR_NAME_MAX + 1);
    ch->carry_note_expression = false;

    Channel_stream_state_reset(ch->csstate);

    Random_reset(&ch->rand);
    Random_reset(&ch->expr_rand);
    if (ch->event_cache != NULL)
        Event_cache_reset(ch->event_cache);

    return;
}


void Channel_apply_defaults(Channel* ch, const Channel_defaults* ch_defaults)
{
    rassert(ch != NULL);
    rassert(ch_defaults != NULL);

    ch->au_input = ch_defaults->control_num;
    strcpy(ch->init_ch_expression, ch_defaults->init_expr);
    bool success = Active_names_set(
            ch->parent.active_names, ACTIVE_CAT_CH_EXPRESSION, ch_defaults->init_expr);
    rassert(success);

    return;
}


void Channel_set_muted(Channel* ch, bool muted)
{
    rassert(ch != NULL);
    ch->mute = muted;
    return;
}


bool Channel_is_muted(const Channel* ch)
{
    rassert(ch != NULL);
    return ch->mute;
}


Random* Channel_get_random_source(Channel* ch)
{
    rassert(ch != NULL);
    return &ch->rand;
}


double Channel_get_fg_force(const Channel* ch)
{
    rassert(ch != NULL);

    if (ch->fg_group_id == 0)
        return NAN;

    return ch->force_controls.force;
}


void Channel_reset_test_output(Channel* ch)
{
    rassert(ch != NULL);

    ch->use_test_output = false;
    ch->test_proc_index = -1;
    memset(ch->test_proc_param, '\0', KQT_VAR_NAME_MAX + 1);

    return;
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


