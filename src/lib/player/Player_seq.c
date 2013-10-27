

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


#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <expr.h>
#include <player/Player_seq.h>
#include <string_common.h>
#include <xassert.h>


bool get_event_type_info(
        Streader* desc_reader,
        const Event_names* names,
        char* ret_name,
        Event_type* ret_type)
{
    assert(desc_reader != NULL);
    assert(names != NULL);
    assert(ret_name != NULL);
    assert(ret_type != NULL);

    if (Streader_is_error_set(desc_reader))
        return false;

    // Read event name
    if (!Streader_readf(desc_reader, "[%s,", EVENT_NAME_MAX, ret_name))
        return false;

    // Check event type
    *ret_type = Event_names_get(names, ret_name);
    if (*ret_type == Event_NONE)
    {
        Streader_set_error(
                desc_reader,
                "Unsupported event type: %s",
                ret_name);
        return false;
    }

    assert(Event_is_valid(*ret_type));
    return true;
}


static bool process_expr(
        Streader* expr_reader,
        Value_type field_type,
        Env_state* estate,
        Random* random,
        const Value* meta,
        Value* ret_value)
{
    assert(expr_reader != NULL);
    assert(estate != NULL);
    assert(random != NULL);
    assert(ret_value != NULL);

    if (Streader_is_error_set(expr_reader))
        return false;

    if (field_type == VALUE_TYPE_NONE)
    {
        ret_value->type = VALUE_TYPE_NONE;
        Streader_read_null(expr_reader);
    }
    else
    {
        evaluate_expr(
                expr_reader,
                estate,
                meta,
                ret_value,
                random);
        Streader_match_char(expr_reader, '"');

        if (Streader_is_error_set(expr_reader))
            return false;

        if (!Value_convert(ret_value, ret_value, field_type))
        {
            Streader_set_error(expr_reader, "Type mismatch");
            return false;
        }
    }

    return true;
}


static void Player_process_expr_event(
        Player* player,
        int ch_num,
        char* trigger_desc,
        const Value* meta,
        bool skip);


void Player_process_event(
        Player* player,
        int ch_num,
        const char* event_name,
        Value* arg,
        bool skip)
{
    assert(player != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(event_name != NULL);
    assert(arg != NULL);

    if (!Event_handler_trigger(
            player->event_handler,
            ch_num,
            event_name,
            arg))
    {
        fprintf(stderr, "`%s` not triggered\n", event_name);
        return;
    }

    if (!skip)
        Event_buffer_add(player->event_buffer, ch_num, event_name, arg);

    // Handle bind
    if (player->module->bind != NULL)
    {
        Target_event* bound = Bind_get_first(
                player->module->bind,
                player->channels[ch_num]->event_cache,
                player->estate,
                event_name,
                arg,
                player->channels[ch_num]->rand);
        while (bound != NULL)
        {
            Player_process_expr_event(
                    player,
                    (ch_num + bound->ch_offset + KQT_CHANNELS_MAX) %
                        KQT_CHANNELS_MAX,
                    bound->desc,
                    arg,
                    skip);

            bound = bound->next;
        }
    }

    return;
}


static void Player_process_expr_event(
        Player* player,
        int ch_num,
        char* trigger_desc,
        const Value* meta,
        bool skip)
{
    assert(player != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(trigger_desc != NULL);

    Streader* sr = Streader_init(
            STREADER_AUTO, trigger_desc, strlen(trigger_desc));

    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[EVENT_NAME_MAX + 1] = "";
    Event_type type = Event_NONE;

    get_event_type_info(
            sr,
            event_names,
            event_name,
            &type);

    Value* arg = VALUE_AUTO;

    if (string_has_suffix(event_name, "\""))
    {
        if (Event_names_get_param_type(event_names, event_name) ==
                VALUE_TYPE_STRING)
        {
            arg->type = VALUE_TYPE_STRING;
            Streader_read_string(
                    sr,
                    ENV_VAR_NAME_MAX,
                    arg->value.string_type);
        }
        else
        {
            fprintf(stderr, "Trigger `%s` has a quote suffix but the"
                    " parameter type is not string", trigger_desc);
            return;
        }
    }
    else
    {
        process_expr(
                sr,
                Event_names_get_param_type(event_names, event_name),
                player->estate,
                player->channels[ch_num]->rand,
                meta,
                arg);
    }

    if (Streader_is_error_set(sr))
    {
        fprintf(stderr,
                "Couldn't parse `%s`: %s\n",
                trigger_desc,
                Streader_get_error_desc(sr));
        return;
    }

    Player_process_event(player, ch_num, event_name, arg, skip);

    return;
}


void Player_process_cgiters(Player* player, Tstamp* limit, bool skip)
{
    assert(player != NULL);
    assert(!Player_has_stopped(player));
    assert(limit != NULL);
    assert(Tstamp_cmp(limit, TSTAMP_AUTO) >= 0);

    // Update current position
    // FIXME: we should really have a well-defined single source of current position
    player->master_params.cur_pos = player->cgiters[0].pos;

    // Stop if we don't have anything to play
    if (player->master_params.cur_pos.piref.pat < 0)
    {
        player->master_params.playback_state = PLAYBACK_STOPPED;
        Tstamp_set(limit, 0, 0);
        return;
    }

    // Find our next jump position
    Tstamp* next_jump_row = Tstamp_set(TSTAMP_AUTO, INT64_MAX, 0);
    int next_jump_ch = KQT_CHANNELS_MAX;
    int next_jump_trigger = INT_MAX;
    Jump_context* next_jc = Active_jumps_get_next_context(
            player->master_params.active_jumps,
            &player->master_params.cur_pos.piref,
            &player->master_params.cur_pos.pat_pos,
            player->master_params.cur_ch,
            player->master_params.cur_trigger);
    if (next_jc != NULL)
    {
        Tstamp_copy(next_jump_row, &next_jc->row);
        next_jump_ch = next_jc->ch_num;
        next_jump_trigger = next_jc->order;
    }

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

                const bool at_active_jump =
                    Tstamp_cmp(next_jump_row, &cgiter->pos.pat_pos) == 0 &&
                    next_jump_ch == i &&
                    next_jump_trigger == player->master_params.cur_trigger;

                if (at_active_jump)
                {
                    // Process our next Jump context
                    assert(next_jc != NULL);
                    if (next_jc->counter > 0)
                    {
                        player->master_params.do_jump = true;
                    }
                    else
                    {
                        // Release our consumed Jump context
                        AAnode* handle = Active_jumps_remove_context(
                                player->master_params.active_jumps, next_jc);
                        Jump_cache_release_context(
                                player->master_params.jump_cache, handle);

                        // Update next Jump context
                        Tstamp_set(next_jump_row, INT64_MAX, 0);
                        next_jump_ch = KQT_CHANNELS_MAX;
                        next_jump_trigger = INT_MAX;
                        next_jc = Active_jumps_get_next_context(
                                player->master_params.active_jumps,
                                &player->master_params.cur_pos.piref,
                                &player->master_params.cur_pos.pat_pos,
                                i,
                                player->master_params.cur_trigger);
                        if (next_jc != NULL)
                        {
                            Tstamp_copy(next_jump_row, &next_jc->row);
                            next_jump_ch = next_jc->ch_num;
                            next_jump_trigger = next_jc->order;
                        }
                    }
                }
                else
                {
                    // Process trigger normally
                    if (!skip ||
                            Event_is_control(event_type) ||
                            Event_is_general(event_type) ||
                            Event_is_master(event_type))
                        Player_process_expr_event(
                                player,
                                i,
                                Event_get_desc(el->event),
                                NULL, // no meta value
                                skip);
                }

                // Perform jump
                if (player->master_params.do_jump)
                {
                    player->master_params.do_jump = false;

                    if (!at_active_jump)
                    {
                        // We just got a new Jump context
                        next_jc = Active_jumps_get_next_context(
                            player->master_params.active_jumps,
                            &player->master_params.cur_pos.piref,
                            &player->master_params.cur_pos.pat_pos,
                            i,
                            player->master_params.cur_trigger);
                        assert(next_jc != NULL);
                    }

                    --next_jc->counter;

                    // Get target pattern instance
                    Pat_inst_ref target_piref = next_jc->target_piref;
                    if (target_piref.pat < 0)
                        target_piref = player->master_params.cur_pos.piref;

                    // Find new track and system
                    Position target_pos;
                    Position_init(&target_pos);
                    if (!Module_find_pattern_location(
                                player->module,
                                &target_piref,
                                &target_pos.track,
                                &target_pos.system))
                    {
                        // Stop if the jump target does not exist
                        player->master_params.playback_state = PLAYBACK_STOPPED;
                    }
                    else
                    {
                        // Move cgiters to the new position
                        Tstamp_copy(&target_pos.pat_pos, &next_jc->target_row);
                        target_pos.piref = next_jc->target_piref;
                        for (int k = 0; k < KQT_CHANNELS_MAX; ++k)
                            Cgiter_reset(
                                    &player->cgiters[k],
                                    &target_pos);
                    }

                    // Make sure all triggers are processed after the jump
                    player->master_params.cur_ch = 0;
                    player->master_params.cur_trigger = 0;

                    Tstamp_set(limit, 0, 0);
                    return;
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

    // TODO: Find our next Jump context

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


