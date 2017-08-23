

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Player_seq.h>

#include <debug/assert.h>
#include <expr.h>
#include <mathnum/common.h>
#include <string/common.h>

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool get_event_type_info(
        Streader* desc_reader,
        const Event_names* names,
        char* ret_name,
        Event_type* ret_type)
{
    rassert(desc_reader != NULL);
    rassert(names != NULL);
    rassert(ret_name != NULL);
    rassert(ret_type != NULL);

    if (Streader_is_error_set(desc_reader))
        return false;

    // Read event name
    if (!Streader_readf(desc_reader, "[%s,", READF_STR(EVENT_NAME_MAX, ret_name)))
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

    rassert(Event_is_valid(*ret_type));
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
    rassert(expr_reader != NULL);
    rassert(estate != NULL);
    rassert(random != NULL);
    rassert(ret_value != NULL);

    if (Streader_is_error_set(expr_reader))
        return false;

    if (field_type == VALUE_TYPE_NONE)
    {
        ret_value->type = VALUE_TYPE_NONE;
        Streader_read_null(expr_reader);
    }
    else
    {
        evaluate_expr(expr_reader, estate, meta, ret_value, random);

        Streader_match_char(expr_reader, '"');

        if (Streader_is_error_set(expr_reader))
            return false;

        if (field_type == VALUE_TYPE_REALTIME)
        {
            if (!Value_type_is_realtime(ret_value->type))
            {
                Streader_set_error(expr_reader, "Type mismatch");
                return false;
            }
        }
        else if (field_type == VALUE_TYPE_MAYBE_STRING)
        {
            if (ret_value->type != VALUE_TYPE_NONE &&
                    ret_value->type != VALUE_TYPE_STRING)
            {
                Streader_set_error(expr_reader, "Type mismatch");
                return false;
            }
        }
        else if (!Value_convert(ret_value, ret_value, field_type))
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
        const char* trigger_desc,
        const Value* meta,
        bool skip,
        bool external);


void Player_process_event(
        Player* player,
        int ch_num,
        const char* event_name,
        const Value* arg,
        bool skip,
        bool external)
{
    rassert(player != NULL);
    rassert(implies(!skip, !Event_buffer_is_full(player->event_buffer)));
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(event_name != NULL);
    rassert(arg != NULL);

    const Event_names* event_names = Event_handler_get_names(player->event_handler);
    const Event_type type = Event_names_get(event_names, event_name);
    rassert(type != Event_NONE);

    if (!Event_is_query(type) &&
            !Event_is_auto(type) &&
            !Event_handler_trigger(
                player->event_handler, ch_num, event_name, arg, external))
    {
        // FIXME: add a proper way of reporting event errors
        fprintf(stderr, "`%s` not fired\n", event_name);
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
                &player->channels[ch_num]->rand);
        while (bound != NULL)
        {
            if (Event_buffer_is_full(player->event_buffer))
            {
                // Set event buffer to skip the amount of events
                // added from the top-level bind
                Event_buffer_start_skipping(player->event_buffer);
                return;
            }

            Player_process_expr_event(
                    player,
                    (ch_num + bound->ch_offset + KQT_CHANNELS_MAX) % KQT_CHANNELS_MAX,
                    bound->desc,
                    arg,
                    skip,
                    external);

            bound = bound->next;
        }
    }

    // Handle query events
    if (!skip && Event_is_query(type))
    {
#define try_process(name, value)                                                   \
        if (true)                                                                  \
        {                                                                          \
            if (Event_buffer_is_full(player->event_buffer))                        \
            {                                                                      \
                Event_buffer_start_skipping(player->event_buffer);                 \
                return;                                                            \
            }                                                                      \
            else                                                                   \
                Player_process_event(player, ch_num, name, value, skip, external); \
        }                                                                          \
        else ignore(0)

        switch (type)
        {
            case Event_query_location:
            {
                const Position* cur_pos = &player->master_params.cur_pos;

                Value* track = VALUE_AUTO;
                track->type = VALUE_TYPE_INT;
                track->value.int_type = cur_pos->track;
                try_process("Atrack", track);

                Value* system = VALUE_AUTO;
                system->type = VALUE_TYPE_INT;
                system->value.int_type = cur_pos->system;
                try_process("Asystem", system);

                if (Position_has_valid_pattern_pos(cur_pos))
                {
                    Value* piref = VALUE_AUTO;
                    piref->type = VALUE_TYPE_PAT_INST_REF;
                    piref->value.Pat_inst_ref_type = cur_pos->piref;
                    try_process("Apattern", piref);
                }

                Value* row = VALUE_AUTO;
                row->type = VALUE_TYPE_TSTAMP;
                row->value.Tstamp_type = cur_pos->pat_pos;
                try_process("Arow", row);
            }
            break;

            case Event_query_voice_count:
            {
                Value* voices = VALUE_AUTO;
                voices->type = VALUE_TYPE_INT;
                voices->value.int_type = player->master_params.active_voices;
                try_process("Avoices", voices);

                Value* vgroups = VALUE_AUTO;
                vgroups->type = VALUE_TYPE_INT;
                vgroups->value.int_type = player->master_params.active_vgroups;
                try_process("Avgroups", vgroups);

                player->master_params.active_voices = 0;
                player->master_params.active_vgroups = 0;
            }
            break;

            case Event_query_actual_force:
            {
                Value* force = VALUE_AUTO;
                force->type = VALUE_TYPE_FLOAT;
                force->value.float_type = Channel_get_fg_force(player->channels[ch_num]);
                if (!isfinite(force->value.float_type))
                {
                    force->type = VALUE_TYPE_BOOL;
                    force->value.bool_type = false;
                }

                try_process("Af", force);
            }
            break;

            default:
                rassert(false);
        }

#undef try_process
    }

    return;
}


static void Player_process_expr_event(
        Player* player,
        int ch_num,
        const char* trigger_desc,
        const Value* meta,
        bool skip,
        bool external)
{
    rassert(player != NULL);
    rassert(implies(!skip, !Event_buffer_is_full(player->event_buffer)));
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(trigger_desc != NULL);

    Streader* sr =
        Streader_init(STREADER_AUTO, trigger_desc, (int64_t)strlen(trigger_desc));

    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[EVENT_NAME_MAX + 1] = "";
    Event_type type = Event_NONE;

    get_event_type_info(sr, event_names, event_name, &type);

    Value* arg = VALUE_AUTO;

    if (string_has_suffix(event_name, "\""))
    {
        if (Event_names_get_param_type(event_names, event_name) == VALUE_TYPE_STRING)
        {
            arg->type = VALUE_TYPE_STRING;
            Streader_read_string(sr, KQT_VAR_NAME_MAX, arg->value.string_type);
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
                &player->channels[ch_num]->rand,
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

    if (!Event_is_control(type) || player->master_params.is_infinite)
        Player_process_event(player, ch_num, event_name, arg, skip, external);

    if (Event_buffer_is_full(player->event_buffer))
        return;

    return;
}


void Player_reset_channels(Player* player)
{
    // Reset channels
    const Channel_defaults_list* ch_defs = Module_get_ch_defaults_list(player->module);

    if (ch_defs != NULL)
    {
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            Channel_reset(player->channels[i]);
            Channel_apply_defaults(
                    player->channels[i], Channel_defaults_list_get(ch_defs, i));
        }
    }
    else
    {
        const Channel_defaults* def_ch_defs =
            Channel_defaults_init(CHANNEL_DEFAULTS_AUTO);
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            Channel_reset(player->channels[i]);
            Channel_apply_defaults(player->channels[i], def_ch_defs);
        }
    }

    return;
}


static void Player_start_pattern_playback_mode(Player* player)
{
    rassert(player != NULL);
    rassert(player->master_params.pattern_playback_flag);
    rassert(player->master_params.playback_state == PLAYBACK_PATTERN);

    player->master_params.pattern_playback_flag = false;

    // Apply channel defaults of the containing song
    int track = -1;
    int system = -1;
    Module_find_pattern_location(
            player->module, &player->master_params.cur_pos.piref, &track, &system);
    Player_reset_channels(player);

    // Move cgiters to the new pattern
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);

    return;
}


static void Player_set_new_playback_position(
        Player* player, const Pat_inst_ref* target_piref, const Tstamp* target_row)
{
    rassert(player != NULL);
    rassert(target_piref != NULL);
    rassert(target_row != NULL);

    Pat_inst_ref actual_target_piref = *target_piref;
    Tstamp* actual_target_row = Tstamp_copy(TSTAMP_AUTO, target_row);

    if (player->master_params.playback_state == PLAYBACK_PATTERN)
    {
        // Don't jump outside the pattern instance
        // in pattern playback mode
        if (actual_target_piref.pat != player->master_params.cur_pos.piref.pat ||
                actual_target_piref.inst != player->master_params.cur_pos.piref.inst)
        {
            actual_target_piref = player->master_params.cur_pos.piref;
            Tstamp_set(actual_target_row, 0, 0);
        }
    }

    // Find new track and system
    Position target_pos;
    Position_init(&target_pos);
    if (!Module_find_pattern_location(
                player->module,
                &actual_target_piref,
                &target_pos.track,
                &target_pos.system))
    {
        // Stop if the jump target does not exist
        player->master_params.playback_state = PLAYBACK_STOPPED;
    }
    else
    {
        // Move cgiters to the new position
        Tstamp_copy(&target_pos.pat_pos, actual_target_row);
        target_pos.piref = actual_target_piref;

        if (player->master_params.playback_state == PLAYBACK_PATTERN)
        {
            target_pos.track = -1;
            target_pos.system = -1;
        }

        for (int k = 0; k < KQT_CHANNELS_MAX; ++k)
            Cgiter_reset(&player->cgiters[k], &target_pos);

        // Set the new position as a global reference
        player->master_params.cur_pos = target_pos;
    }

    // Make sure all triggers are processed after the jump
    player->master_params.cur_ch = 0;
    player->master_params.cur_trigger = 0;

    return;
}


bool Player_check_perform_goto(Player* player)
{
    if (!player->master_params.do_goto)
        return false;

    player->master_params.do_goto = false;

    // Get target pattern instance
    Pat_inst_ref target_piref = player->master_params.goto_target_piref;
    if (target_piref.pat < 0)
        target_piref = player->master_params.cur_pos.piref;

    // Get target row
    Tstamp* target_row = TSTAMP_AUTO;
    Tstamp_copy(target_row, &player->master_params.goto_target_row);

    Player_set_new_playback_position(player, &target_piref, target_row);

    return true;
}


void Player_process_cgiters(Player* player, Tstamp* limit, bool skip)
{
    rassert(player != NULL);
    rassert(!Player_has_stopped(player));
    rassert(limit != NULL);
    rassert(Tstamp_cmp(limit, TSTAMP_AUTO) >= 0);

    // Check pattern playback start
    if (player->master_params.pattern_playback_flag)
        Player_start_pattern_playback_mode(player);

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
    int64_t next_jump_trigger = INT64_MAX;
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
            rassert(tr->head->next != NULL);
            Trigger_list* trl = tr->head->next;

            // Skip triggers if resuming
            int trigger_index = 0;
            while (trigger_index < player->master_params.cur_trigger &&
                    trl->trigger != NULL)
            {
                ++trigger_index;
                trl = trl->next;
            }

            // Process triggers
            while (trl->trigger != NULL)
            {
                const Event_type event_type = Trigger_get_type(trl->trigger);

                const bool at_active_jump =
                    Tstamp_cmp(next_jump_row, &cgiter->pos.pat_pos) == 0 &&
                    next_jump_ch == i &&
                    next_jump_trigger == player->master_params.cur_trigger;

                if (at_active_jump)
                {
                    // Process our next Jump context
                    rassert(next_jc != NULL);
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
                        next_jump_trigger = INT64_MAX;
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
                    {
                        if (!Event_is_control(event_type) ||
                                player->master_params.is_infinite)
                        {
                            // Break if event buffer is full
                            if (!skip && Event_buffer_is_full(player->event_buffer))
                            {
                                Tstamp_set(limit, 0, 0);

                                // Make sure we get this row again next time
                                Cgiter_clear_returned_status(cgiter);
                                return;
                            }

                            const bool external = false;

                            Player_process_expr_event(
                                    player,
                                    i,
                                    Trigger_get_desc(trl->trigger),
                                    NULL, // no meta value
                                    skip,
                                    external);

                            // Break if started event skipping
                            if (Event_buffer_is_skipping(player->event_buffer))
                            {
                                rassert(Event_buffer_is_full(player->event_buffer));
                                Tstamp_set(limit, 0, 0);

                                // Make sure we get this row again next time
                                Cgiter_clear_returned_status(cgiter);
                                return;
                            }

                            // Event fully processed
                            Event_buffer_reset_add_counter(player->event_buffer);
                        }
                    }
                }

                // Check pattern playback start
                if (player->master_params.pattern_playback_flag)
                    Player_start_pattern_playback_mode(player);

                // Perform goto
                if (Player_check_perform_goto(player))
                {
                    Tstamp_set(limit, 0, 0);
                    return;
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
                        rassert(next_jc != NULL);
                    }

                    --next_jc->counter;

                    // Get target pattern instance
                    Pat_inst_ref target_piref = next_jc->target_piref;
                    if (target_piref.pat < 0)
                        target_piref = player->master_params.cur_pos.piref;

                    // Get target row
                    Tstamp* target_row = TSTAMP_AUTO;
                    Tstamp_copy(target_row, &next_jc->target_row);

                    Player_set_new_playback_position(player, &target_piref, target_row);

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

                trl = trl->next;
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

    if (Tstamp_cmp(&master_params->tempo_slide_left, TSTAMP_AUTO) <= 0)
    {
        // Finish slide
        master_params->tempo = master_params->tempo_slide_target;
        master_params->tempo_slide = 0;
    }
    else if (Tstamp_cmp(&master_params->tempo_slide_slice_left, TSTAMP_AUTO) <= 0)
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

    return;
}


void Player_update_sliders_and_lfos_tempo(Player* player)
{
    rassert(player != NULL);

    const double tempo = player->master_params.tempo;
    rassert(isfinite(tempo));

    Master_params* mp = &player->master_params;
    Slider_set_tempo(&mp->volume_slider, tempo);

    // Update channels
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel* ch = player->channels[i];
        Channel_set_tempo(ch, tempo);
    }

    // Update devices
    Device_states_set_tempo(player->device_states, player->master_params.tempo);

    return;
}


int32_t Player_move_forwards(Player* player, int32_t nframes, bool skip)
{
    rassert(player != NULL);
    rassert(!Player_has_stopped(player));
    rassert(nframes >= 0);

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
            TSTAMP_AUTO, nframes, player->master_params.tempo, player->audio_rate);

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

    if (Tstamp_cmp(limit, TSTAMP_AUTO) > 0)
    {
        // We are moving forwards, so reset our goto safety counter
        player->master_params.goto_safety_counter = 0;
    }

    // Get actual number of frames to be rendered
    double dframes =
        Tstamp_toframes(limit, player->master_params.tempo, player->audio_rate);
    rassert(dframes >= 0.0);

    int32_t to_be_rendered = (int32_t)dframes;
    player->frame_remainder += dframes - to_be_rendered;
    if (player->frame_remainder > 0.5)
    {
        ++to_be_rendered;
        player->frame_remainder -= 1.0;
    }

    rassert(to_be_rendered <= nframes);

    return to_be_rendered;
}


