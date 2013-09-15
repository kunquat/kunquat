

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdio.h>

#include <events/Event_master_jump.h>
#include <expr.h>
#include <player/Player_seq.h>
#include <string_common.h>
#include <xassert.h>


char* get_event_type_info(
        char* desc,
        const Event_names* names,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type)
{
    assert(desc != NULL);
    assert(names != NULL);
    assert(rs != NULL);
    assert(ret_name != NULL);
    assert(ret_type != NULL);

    if (rs->error)
        return desc;

    // Read event name
    desc = read_const_char(desc, '[', rs);
    desc = read_string(desc, ret_name, EVENT_NAME_MAX, rs);
    desc = read_const_char(desc, ',', rs);
    if (rs->error)
        return desc;

    // Check event type
    *ret_type = Event_names_get(names, ret_name);
    if (*ret_type == Event_NONE)
    {
        Read_state_set_error(
                rs,
                "Unsupported event type: %s",
                ret_name);
        return desc;
    }

    assert(Event_is_valid(*ret_type));
    return desc;
}


static char* process_expr(
        char* arg_expr,
        Value_type field_type,
        Env_state* estate,
        Random* random,
        const Value* meta,
        Read_state* rs,
        Value* ret_value)
{
    assert(arg_expr != NULL);
    assert(estate != NULL);
    assert(random != NULL);
    assert(rs != NULL);
    assert(ret_value != NULL);

    if (rs->error)
        return arg_expr;

    if (field_type == VALUE_TYPE_NONE)
    {
        ret_value->type = VALUE_TYPE_NONE;
        arg_expr = read_null(arg_expr, rs);
    }
    else
    {
        arg_expr = evaluate_expr(
                arg_expr,
                estate,
                rs,
                meta,
                ret_value,
                random);
        arg_expr = read_const_char(arg_expr, '"', rs);

        if (rs->error)
            return arg_expr;

        if (!Value_convert(ret_value, ret_value, field_type))
            Read_state_set_error(rs, "Type mismatch");
    }

    return arg_expr;
}


void Player_process_trigger(
        Player* player,
        int ch_num,
        char* trigger_desc,
        bool skip)
{
    assert(player != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(trigger_desc != NULL);

    Read_state* rs = READ_STATE_AUTO;
    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[EVENT_NAME_MAX + 1] = "";
    Event_type type = Event_NONE;

    char* str_pos = get_event_type_info(
            trigger_desc,
            event_names,
            rs,
            event_name,
            &type);

    Value* arg = VALUE_AUTO;

    if (string_has_suffix(event_name, "\""))
    {
        if (Event_names_get_param_type(event_names, event_name) ==
                VALUE_TYPE_STRING)
        {
            arg->type = VALUE_TYPE_STRING;
            str_pos = read_string(
                    str_pos,
                    arg->value.string_type,
                    ENV_VAR_NAME_MAX,
                    rs);
        }
        else
        {
            fprintf(stderr, "Trigger `%s` has a quote suffix but the"
                    " parameter type is not string", trigger_desc);
            return;
        }
    }
    else
        str_pos = process_expr(
                str_pos,
                Event_names_get_param_type(event_names, event_name),
                player->estate,
                player->channels[ch_num]->rand,
                NULL,
                rs,
                arg);

    if (rs->error)
    {
        fprintf(stderr, "Couldn't parse `%s`: %s\n", trigger_desc, rs->message);
        return;
    }

    if (!Event_handler_trigger(
            player->event_handler,
            ch_num,
            event_name,
            arg))
    {
        fprintf(stderr, "`%s` not triggered\n", trigger_desc);
        return;
    }

    if (!skip)
        Event_buffer_add(player->event_buffer, ch_num, event_name, arg);

    return;
}


void Player_process_cgiters(Player* player, Tstamp* limit, bool skip)
{
    assert(player != NULL);
    assert(!Player_has_stopped(player));
    assert(limit != NULL);
    assert(Tstamp_cmp(limit, TSTAMP_AUTO) >= 0);

    // Process trigger rows at current position
    for (int i = player->master_params.cur_ch; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter* cgiter = &player->cgiters[i];

        if (Cgiter_has_finished(cgiter)) // implies empty playback
            break;

        const Trigger_row* tr = Cgiter_get_trigger_row(cgiter);
        if (tr != NULL)
        {
            // Process trigger row
            assert(tr->head->next != NULL);
            Event_list* el = tr->head->next;

            // Skip triggers if resuming
            int trigger_index = 0;
            while (trigger_index < player->master_params.cur_trigger &&
                    el->event != NULL)
            {
                ++trigger_index;
                el = el->next;
            }

            // Process triggers
            while (el->event != NULL)
            {
                const Event_type event_type = Event_get_type(el->event);

                if (event_type == Trigger_jump)
                {
                    // Set current pattern instance, FIXME: hackish
                    player->master_params.cur_pos.piref =
                        cgiter->pos.piref;

                    Trigger_master_jump_process(
                            el->event,
                            &player->master_params);

                    // Break if jump triggered
                    if (player->master_params.do_jump)
                    {
                        player->master_params.do_jump = false;
                        Tstamp_set(limit, 0, 0);

                        // Move cgiters to the new position
                        for (int k = 0; k < KQT_CHANNELS_MAX; ++k)
                            Cgiter_reset(
                                    &player->cgiters[k],
                                    &player->master_params.cur_pos);

                        return;
                    }
                }
                else
                {
                    if (!skip ||
                            Event_is_control(event_type) ||
                            Event_is_general(event_type) ||
                            Event_is_master(event_type))
                        Player_process_trigger(
                                player,
                                i,
                                Event_get_desc(el->event),
                                skip);
                }

                ++player->master_params.cur_trigger;

                // Break if delay was added
                if (Tstamp_cmp(&player->master_params.delay_left,
                            TSTAMP_AUTO) > 0)
                {
                    Tstamp_set(limit, 0, 0);

                    // Make sure we get this row again next time
                    Cgiter_clear_returned_status(cgiter);
                    return;
                }

                el = el->next;
            }
        }

        // All triggers processed in this column
        player->master_params.cur_trigger = 0;
        ++player->master_params.cur_ch;

        // See how much we can move forwards
        Tstamp* dist = Tstamp_copy(TSTAMP_AUTO, limit);
        if (Cgiter_peek(cgiter, dist) && Tstamp_cmp(dist, limit) < 0)
            Tstamp_copy(limit, dist);
    }

    // All trigger rows processed
    player->master_params.cur_ch = 0;
    player->master_params.cur_trigger = 0;

    // Break if tempo settings changed
    if (player->master_params.tempo_settings_changed)
    {
        Tstamp_set(limit, 0, 0);
        return;
    }

    bool any_cgiter_active = false;

    // Move cgiters forwards and check for playback end
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter* cgiter = &player->cgiters[i];
        Cgiter_move(cgiter, limit);
        any_cgiter_active |= !Cgiter_has_finished(cgiter);
    }

    // Stop if all cgiters have finished
    if (!any_cgiter_active)
    {
        // TODO: safety check for zero-length playback!
        if (player->master_params.is_infinite)
        {
#if 0
            fprintf(stderr, "Resetting to %d %d " PRIts " %d %d\n",
                    (int)player->master_params.start_pos.track,
                    (int)player->master_params.start_pos.system,
                    PRIVALts(player->master_params.start_pos.pat_pos),
                    (int)player->master_params.start_pos.piref.pat,
                    (int)player->master_params.start_pos.piref.inst);
#endif
            for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
            {
                Cgiter_reset(
                        &player->cgiters[i],
                        &player->master_params.start_pos);
            }
        }
        else
        {
            player->master_params.playback_state = PLAYBACK_STOPPED;
        }

        Tstamp_set(limit, 0, 0);
        return;
    }

    return;
}


static void update_tempo_slide(Master_params* master_params)
{
    if (master_params->tempo_slide == 0)
        return;

    if (Tstamp_cmp(
                &master_params->tempo_slide_left,
                TSTAMP_AUTO) <= 0)
    {
        // Finish slide
        master_params->tempo = master_params->tempo_slide_target;
        master_params->tempo_slide = 0;
    }
    else if (Tstamp_cmp(
                &master_params->tempo_slide_slice_left,
                TSTAMP_AUTO) <= 0)
    {
        // New tempo
        master_params->tempo += master_params->tempo_slide_update;
        master_params->tempo_settings_changed = true;

        const bool is_too_low = master_params->tempo_slide < 0 &&
            master_params->tempo < master_params->tempo_slide_target;
        const bool is_too_high = master_params->tempo_slide > 0 &&
            master_params->tempo > master_params->tempo_slide_target;
        if (is_too_low || is_too_high)
        {
            // Finish slide
            master_params->tempo = master_params->tempo_slide_target;
            master_params->tempo_slide = 0;
        }
        else
        {
            // Start next slice
            Tstamp_set(
                    &master_params->tempo_slide_slice_left,
                    0, KQT_TEMPO_SLIDE_SLICE_LEN);
            Tstamp_mina(
                    &master_params->tempo_slide_slice_left,
                    &master_params->tempo_slide_left);
        }
    }
}


void Player_update_sliders_and_lfos_tempo(Player* player)
{
    assert(player != NULL);

    const double tempo = player->master_params.tempo;
    assert(isfinite(tempo));

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel* ch = player->channels[i];
        LFO_set_tempo(&ch->vibrato, tempo);
        LFO_set_tempo(&ch->tremolo, tempo);
        Slider_set_tempo(&ch->panning_slider, tempo);
        LFO_set_tempo(&ch->autowah, tempo);
    }

    Master_params* mp = &player->master_params;
    Slider_set_tempo(&mp->volume_slider, tempo);

    return;
}


int32_t Player_move_forwards(Player* player, int32_t nframes, bool skip)
{
    assert(player != NULL);
    assert(!Player_has_stopped(player));
    assert(nframes >= 0);

    // Process tempo
    update_tempo_slide(&player->master_params);
    if (player->master_params.tempo_settings_changed)
    {
        player->master_params.tempo_settings_changed = false;

        Player_update_sliders_and_lfos_tempo(player);
    }

    /*
    fprintf(stderr, "Tempo: %.2f %d " PRIts " %.2f " PRIts " " PRIts " %.2f\n",
            player->master_params.tempo,
            player->master_params.tempo_slide,
            PRIVALts(player->master_params.tempo_slide_length),
            player->master_params.tempo_slide_target,
            PRIVALts(player->master_params.tempo_slide_left),
            PRIVALts(player->master_params.tempo_slide_slice_left),
            player->master_params.tempo_slide_update);
    // */

    // Get maximum duration to move forwards
    Tstamp* limit = Tstamp_fromframes(
            TSTAMP_AUTO,
            nframes,
            player->master_params.tempo,
            player->audio_rate);

    if (player->master_params.tempo_slide != 0)
    {
        // Apply tempo slide slice
        Tstamp_mina(limit, &player->master_params.tempo_slide_slice_left);
        Tstamp_suba(&player->master_params.tempo_slide_slice_left, limit);
    }

    Tstamp* delay_left = &player->master_params.delay_left;

    if (Tstamp_cmp(delay_left, TSTAMP_AUTO) > 0)
    {
        // Apply pattern delay
        Tstamp_mina(limit, delay_left);
        Tstamp_suba(delay_left, limit);
    }
    else
    {
        // Process cgiters
        Player_process_cgiters(player, limit, skip);
    }

    // Get actual number of frames to be rendered
    double dframes = Tstamp_toframes(
            limit,
            player->master_params.tempo,
            player->audio_rate);
    assert(dframes >= 0.0);

    int32_t to_be_rendered = (int32_t)dframes;
    player->frame_remainder += dframes - to_be_rendered;
    if (player->frame_remainder > 0.5)
    {
        ++to_be_rendered;
        player->frame_remainder -= 1.0;
    }

    assert(to_be_rendered <= nframes);

    return to_be_rendered;
}


