

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Player.h>

#include <debug/assert.h>
#include <init/devices/Au_params.h>
#include <init/devices/Audio_unit.h>
#include <init/sheet/Channel_defaults.h>
#include <mathnum/common.h>
#include <memory.h>
#include <Pat_inst_ref.h>
#include <player/devices/Voice_state.h>
#include <player/Player_private.h>
#include <player/Player_seq.h>
#include <player/Position.h>
#include <player/Tuning_state.h>
#include <player/Voice_group.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void Player_update_sliders_and_lfos_audio_rate(Player* player)
{
    rassert(player != NULL);

    const int32_t rate = player->audio_rate;

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel* ch = player->channels[i];
        Channel_set_audio_rate(ch, rate);
    }

    Master_params* mp = &player->master_params;
    Slider_set_audio_rate(&mp->volume_slider, rate);

    return;
}


Player* new_Player(
        const Module* module,
        int32_t audio_rate,
        int32_t audio_buffer_size,
        int32_t event_buffer_size,
        int voice_count)
{
    rassert(module != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);
    rassert(audio_buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    rassert(voice_count >= 0);
    rassert(voice_count <= KQT_VOICES_MAX);

    Player* player = memory_alloc_item(Player);
    if (player == NULL)
        return NULL;

    // Sanitise fields
    player->module = module;

    player->audio_rate = audio_rate;
    player->audio_buffer_size = audio_buffer_size;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        player->audio_buffers[i] = NULL;
    player->audio_frames_available = 0;

    player->device_states = NULL;
    player->estate = NULL;
    player->event_buffer = NULL;
    player->voices = NULL;
    player->work_buffers = NULL;
    Master_params_preinit(&player->master_params);
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        player->channels[i] = NULL;
    player->event_handler = NULL;

    player->frame_remainder = 0.0;

    player->cgiters_accessed = false;
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_init(&player->cgiters[i], player->module, i);

    player->audio_frames_processed = 0;
    player->nanoseconds_history = 0;

    player->events_returned = false;

    player->susp_event_ch = -1;
    memset(player->susp_event_name, '\0', EVENT_NAME_MAX + 1);
    player->susp_event_value = *VALUE_AUTO;

    // Init fields
    player->device_states = new_Device_states();
    player->estate = new_Env_state(player->module->env);
    player->event_buffer = new_Event_buffer(event_buffer_size);
    player->voices = new_Voice_pool(voice_count);
    if (player->device_states == NULL ||
            player->estate == NULL ||
            player->event_buffer == NULL ||
            player->voices == NULL ||
            !Voice_pool_reserve_state_space(
                player->voices,
                sizeof(Voice_state)))
    {
        del_Player(player);
        return NULL;
    }

    Device_state* master_state = Device_create_state(
            (const Device*)player->module,
            player->audio_rate,
            player->audio_buffer_size);
    if (master_state == NULL || !Device_states_add_state(
                player->device_states, master_state))
    {
        del_Device_state(master_state);
        del_Player(player);
        return NULL;
    }

    player->work_buffers = new_Work_buffers(player->audio_buffer_size);
    if (player->work_buffers == NULL)
    {
        del_Player(player);
        return NULL;
    }

    if (Master_params_init(
                &player->master_params, player->module, player->estate) == NULL)
    {
        del_Player(player);
        return NULL;
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        player->channels[i] = new_Channel(
                player->module,
                i,
                Module_get_au_table(player->module),
                player->estate,
                player->voices,
                player->master_params.tempo,
                player->audio_rate);
        if (player->channels[i] == NULL)
        {
            del_Player(player);
            return NULL;
        }
    }

    Player_update_sliders_and_lfos_audio_rate(player);

    player->event_handler = new_Event_handler(
            &player->master_params,
            player->channels,
            player->device_states,
            Module_get_au_table(player->module));
    if (player->event_handler == NULL)
    {
        del_Player(player);
        return NULL;
    }

    if (player->audio_buffer_size > 0)
    {
        for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        {
            player->audio_buffers[i] = memory_alloc_items(
                    float, player->audio_buffer_size);
            if (player->audio_buffers[i] == NULL)
            {
                del_Player(player);
                return NULL;
            }
        }
    }

    return player;
}


const Event_handler* Player_get_event_handler(const Player* player)
{
    rassert(player != NULL);
    return player->event_handler;
}


Device_states* Player_get_device_states(const Player* player)
{
    rassert(player != NULL);
    return player->device_states;
}


bool Player_reserve_voice_state_space(Player* player, int32_t size)
{
    rassert(player != NULL);
    rassert(size >= 0);

    return Voice_pool_reserve_state_space(player->voices, size);
}


int32_t Player_get_voice_work_buffer_size(const Player* player)
{
    rassert(player != NULL);
    return Voice_pool_get_work_buffer_size(player->voices);
}


bool Player_reserve_voice_work_buffer_space(Player* player, int32_t size)
{
    rassert(player != NULL);
    rassert(size >= 0);
    rassert(size <= VOICE_WORK_BUFFER_SIZE_MAX);

    return Voice_pool_reserve_work_buffers(player->voices, size);
}


bool Player_alloc_channel_cv_state(Player* player, const Au_control_vars* aucv)
{
    rassert(player != NULL);
    rassert(aucv != NULL);

    Au_control_var_iter* iter = Au_control_var_iter_init(AU_CONTROL_VAR_ITER_AUTO, aucv);
    const char* var_name = NULL;
    Value_type var_type = VALUE_TYPE_NONE;
    Au_control_var_iter_get_next_var_info(iter, &var_name, &var_type);
    while (var_name != NULL)
    {
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            if (!Channel_cv_state_add_entry(player->channels[i]->cvstate, var_name))
                return false;
        }

        Au_control_var_iter_get_next_var_info(iter, &var_name, &var_type);
    }

    return true;
}


bool Player_alloc_channel_streams(Player* player, const Au_streams* streams)
{
    rassert(player != NULL);
    rassert(streams != NULL);

    Stream_target_dev_iter* iter =
        Stream_target_dev_iter_init(STREAM_TARGET_DEV_ITER_AUTO, streams);

    const char* name = Stream_target_dev_iter_get_next(iter);
    while (name != NULL)
    {
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            if (!Channel_stream_state_add_entry(player->channels[i]->csstate, name))
                return false;
        }

        name = Stream_target_dev_iter_get_next(iter);
    }

    return true;
}


bool Player_refresh_env_state(Player* player)
{
    rassert(player != NULL);
    return Env_state_refresh_space(player->estate);
}


bool Player_refresh_bind_state(Player* player)
{
    rassert(player != NULL);

    Event_cache* caches[KQT_CHANNELS_MAX] = { NULL };
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        caches[i] = Bind_create_cache(player->module->bind);
        if (caches[i] == NULL)
        {
            for (int k = i - 1; k >= 0; --k)
                del_Event_cache(caches[k]);

            return false;
        }
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Channel_set_event_cache(player->channels[i], caches[i]);

    return true;
}


bool Player_create_tuning_state(Player* player, int index)
{
    rassert(player != NULL);
    rassert(index >= 0);
    rassert(index < KQT_TUNING_TABLES_MAX);

    if (player->master_params.tuning_states[index] == NULL)
    {
        player->master_params.tuning_states[index] = new_Tuning_state();
        if (player->master_params.tuning_states[index] == NULL)
            return false;
    }

    Tuning_state_reset(
            player->master_params.tuning_states[index],
            Module_get_tuning_table(player->module, index));

    return true;
}


void Player_reset(Player* player, int track_num)
{
    rassert(player != NULL);
    rassert(track_num >= -1);
    rassert(track_num < KQT_TRACKS_MAX);

    Master_params_reset(&player->master_params);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Channel_set_random_seed(player->channels[i], player->module->random_seed);

    Player_update_sliders_and_lfos_audio_rate(player);
    Player_update_sliders_and_lfos_tempo(player);

    player->frame_remainder = 0.0;

    Player_reset_channels(player);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);

    player->cgiters_accessed = false;

    Event_buffer_clear(player->event_buffer);

    player->audio_frames_processed = 0;
    player->nanoseconds_history = 0;

    player->events_returned = false;

    Env_state_reset(player->estate);

    Voice_pool_reset(player->voices);

    return;
}


void Player_reset_dc_blocker(Player* player)
{
    rassert(player != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        player->master_params.dc_block_state[port].feedforward = 0;
        player->master_params.dc_block_state[port].feedback = 0;
    }

    return;
}


bool Player_set_audio_rate(Player* player, int32_t rate)
{
    rassert(player != NULL);
    rassert(rate > 0);

    if (player->audio_rate == rate)
        return true;

    if (!Device_states_set_audio_rate(player->device_states, rate))
        return false;

    // Resize Voice work buffers
    {
        int32_t voice_wb_size = 0;
        Au_table* au_table = Module_get_au_table(player->module);
        for (int au_i = 0; au_i < KQT_AUDIO_UNITS_MAX; ++au_i)
        {
            const Audio_unit* au = Au_table_get(au_table, au_i);
            if (au != NULL)
            {
                const int32_t au_req_voice_wb_size =
                    Audio_unit_get_voice_wb_size(au, rate);
                voice_wb_size = max(voice_wb_size, au_req_voice_wb_size);
            }
        }

        if (!Player_reserve_voice_work_buffer_space(player, voice_wb_size))
            return false;
    }

    // Add current playback frame count to nanoseconds history
    player->nanoseconds_history +=
        player->audio_frames_processed * 1000000000LL / player->audio_rate;
    player->audio_frames_processed = 0;

    player->audio_rate = rate;

    Player_update_sliders_and_lfos_audio_rate(player);

    return true;
}


int32_t Player_get_audio_rate(const Player* player)
{
    rassert(player != NULL);
    return player->audio_rate;
}


bool Player_set_audio_buffer_size(Player* player, int32_t size)
{
    rassert(player != NULL);
    rassert(size >= 0);

    if (player->audio_buffer_size == size)
        return true;

    // Reduce supported size (in case we fail memory allocation)
    player->audio_buffer_size = min(player->audio_buffer_size, size);

    // Handle empty buffers
    if (player->audio_buffer_size == 0)
    {
        for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        {
            memory_free(player->audio_buffers[i]);
            player->audio_buffers[i] = NULL;
        }
    }
    else
    {
        for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        {
            float* new_buffer =
                memory_realloc_items(float, size, player->audio_buffers[i]);
            if (new_buffer == NULL)
                return false;

            player->audio_buffers[i] = new_buffer;
        }
    }

    // Update device state buffers
    if (!Device_states_set_audio_buffer_size(player->device_states, size))
        return false;

    // Update work buffers
    if (!Work_buffers_resize(player->work_buffers, size))
        return false;

    // Set final supported buffer size
    player->audio_buffer_size = size;

    return true;
}


int32_t Player_get_audio_buffer_size(const Player* player)
{
    rassert(player != NULL);
    return player->audio_buffer_size;
}


int64_t Player_get_nanoseconds(const Player* player)
{
    rassert(player != NULL);

    static const int64_t ns_second = 1000000000LL;

    int64_t ns_this_audio_rate = 0;
    if (INT64_MAX / ns_second < player->audio_frames_processed)
        ns_this_audio_rate = (int64_t)((double)player->audio_frames_processed *
                ((double)ns_second / (double)player->audio_rate));
    else
        ns_this_audio_rate =
            (int64_t)(player->audio_frames_processed * ns_second / player->audio_rate);

    return player->nanoseconds_history + ns_this_audio_rate;
}


static void Player_process_voices(
        Player* player, int32_t render_start, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(render_start >= 0);
    rassert(frame_count >= 0);

    if (frame_count == 0)
        return;

    // Verify foreground voice ownerships
    // TODO: this is required for correct event processing, move elsewhere maybe?
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel* ch = player->channels[i];
        for (int k = 0; k < KQT_PROCESSORS_MAX; ++k)
        {
            if (ch->fg[k] != NULL)
            {
                // Verify voice ownership
                ch->fg[k] = Voice_pool_get_voice(
                        player->voices, ch->fg[k], ch->fg_id[k]);
            }
        }
    }

    // Process active Voice groups
    const int32_t render_stop = render_start + frame_count;
    int active_voice_count = 0;

    Voice_group* vg = Voice_pool_start_group_iteration(player->voices);

    while (vg != NULL)
    {
        // Find the connections that contain the processors
        const Voice* first_voice = Voice_group_get_voice(vg, 0);
        const Processor* first_proc = Voice_get_proc(first_voice);
        const Au_params* first_au_params = Processor_get_au_params(first_proc);
        const uint32_t au_id = first_au_params->device_id;
        const Device_state* au_state = Device_states_get_state(
                player->device_states, au_id);
        const Audio_unit* au = (const Audio_unit*)Device_state_get_device(au_state);
        const Connections* conns = Audio_unit_get_connections(au);

        if (conns != NULL)
        {
            const int32_t process_stop = Connections_process_voice_group(
                    conns,
                    vg,
                    player->device_states,
                    player->work_buffers,
                    render_start,
                    render_stop,
                    player->audio_rate,
                    player->master_params.tempo);

            Connections_mix_voice_signals(
                    conns, vg, player->device_states, render_start, process_stop);

            if (process_stop < render_stop)
                Voice_group_deactivate_all(vg);
            else
                Voice_group_deactivate_unreachable(vg);

            active_voice_count += Voice_group_get_active_count(vg);
        }
        else
        {
            Voice_group_deactivate_all(vg);
        }

        vg = Voice_pool_get_next_group(player->voices);
    }

    player->master_params.active_voices =
        max(player->master_params.active_voices, active_voice_count);
}


static bool Player_update_receive(Player* player)
{
    bool new_events_found = false;

    Event_buffer_clear(player->event_buffer);

    if (Event_buffer_is_skipping(player->event_buffer))
    {
        new_events_found = true;

        // Process suspended bind
        if (string_eq(player->susp_event_name, ""))
        {
            Player_move_forwards(player, 0, false);
        }
        else
        {
            Player_process_event(
                    player,
                    player->susp_event_ch,
                    player->susp_event_name,
                    &player->susp_event_value,
                    false);

            // Check and perform goto if needed
            Player_check_perform_goto(player);
        }

        if (Event_buffer_is_skipping(player->event_buffer))
        {
            return new_events_found;
        }
        else
        {
            player->susp_event_name[0] = '\0';
            Event_buffer_reset_add_counter(player->event_buffer);
        }
    }

    if (player->master_params.cur_ch > 0 ||
            player->master_params.cur_trigger > 0)
    {
        new_events_found = true;

        const int old_ch = player->master_params.cur_ch;
        const int old_trigger = player->master_params.cur_trigger;

        // Process the remainder of the current row
        Player_move_forwards(player, 0, false);

        // Check if we reached end of row
        if (old_ch == player->master_params.cur_ch &&
                old_trigger == player->master_params.cur_trigger &&
                !Event_buffer_is_skipping(player->event_buffer))
            new_events_found = false;
    }

    return new_events_found;
}


static void Player_flush_receive(Player* player)
{
    rassert(player != NULL);

    while (Player_update_receive(player))
        ;

    return;
}


static void Player_apply_dc_blocker(
        Player* player, int32_t buf_start, int32_t buf_stop)
{
    rassert(player != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    // Get access to mixed output
    Device_state* master_state = Device_states_get_state(
            player->device_states, Device_get_id((const Device*)player->module));
    rassert(master_state != NULL);

    // Implementation based on https://ccrma.stanford.edu/~jos/filters/DC_Blocker.html
    static const double adapt_time = 0.01;
    const double adapt_time_frames = max(2, adapt_time * player->audio_rate);
    const float R = (float)((adapt_time_frames - 1) / adapt_time_frames);
    const float gain = (1 + R) / 2;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* buffer = Device_state_get_audio_buffer(
                master_state, DEVICE_PORT_TYPE_RECEIVE, port);
        if (buffer != NULL)
        {
            float feedforward = player->master_params.dc_block_state[port].feedforward;
            float feedback = player->master_params.dc_block_state[port].feedback;

            float* buf = Work_buffer_get_contents_mut(buffer);
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                const float in = buf[i];
                const float out = gain * (in - feedforward) + R * feedback;
                feedforward = in;
                feedback = out;
                buf[i] = out;
            }

            player->master_params.dc_block_state[port].feedforward = feedforward;
            player->master_params.dc_block_state[port].feedback = feedback;
        }
    }

    return;
}


static void Player_apply_master_volume(
        Player* player, int32_t buf_start, int32_t buf_stop)
{
    rassert(player != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= 0);

    static const int CONTROL_WB_MASTER_VOLUME = WORK_BUFFER_IMPL_1;

    float* volumes = Work_buffers_get_buffer_contents_mut(
            player->work_buffers, CONTROL_WB_MASTER_VOLUME);

    if (Slider_in_progress(&player->master_params.volume_slider))
    {
        double final_volume = player->master_params.volume;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            final_volume = Slider_step(&player->master_params.volume_slider);
            volumes[i] = (float)final_volume;
        }
        player->master_params.volume = final_volume;
    }
    else
    {
        const float cur_volume = (float)player->master_params.volume;
        for (int32_t i = buf_start; i < buf_stop; ++i)
            volumes[i] = cur_volume;
    }

    // Get access to mixed output
    Device_state* master_state = Device_states_get_state(
            player->device_states, Device_get_id((const Device*)player->module));
    rassert(master_state != NULL);

    for (int32_t port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Work_buffer* buffer = Device_state_get_audio_buffer(
                master_state, DEVICE_PORT_TYPE_RECEIVE, port);
        if (buffer != NULL)
        {
            float* buf = Work_buffer_get_contents_mut(buffer);
            for (int32_t i = buf_start; i < buf_stop; ++i)
                buf[i] *= volumes[i];
        }
    }

    return;
}


static void Player_init_final(Player* player)
{
    rassert(player != NULL);

    Master_params_set_starting_tempo(&player->master_params);

    Device_states_set_tempo(player->device_states, player->master_params.tempo);

    // Init control variables
    {
        Au_table* aus = Module_get_au_table(player->module);
        for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
        {
            const Audio_unit* au = Au_table_get(aus, i);
            if (au != NULL)
                Device_init_control_vars(
                        (const Device*)au,
                        player->device_states,
                        DEVICE_CONTROL_VAR_MODE_MIXED,
                        &player->master_params.random,
                        NULL);
        }
    }

    Player_reset_channels(player);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);

    return;
}


void Player_play(Player* player, int32_t nframes)
{
    rassert(player != NULL);
    rassert(player->audio_buffer_size > 0);
    rassert(nframes >= 0);

    Player_flush_receive(player);

    Event_buffer_clear(player->event_buffer);

    nframes = min(nframes, player->audio_buffer_size);

    const Connections* connections = Module_get_connections(player->module);
    rassert(connections != NULL);

    Device_states_clear_audio_buffers(player->device_states, 0, nframes);
    Connections_clear_buffers(connections, player->device_states, 0, nframes);

    // TODO: check if song or pattern instance location has changed

    // Composition-level progress
    const bool was_playing = !Player_has_stopped(player);
    int32_t rendered = 0;
    while (rendered < nframes && !Event_buffer_is_full(player->event_buffer))
    {
        // Move forwards in composition
        int32_t to_be_rendered = nframes - rendered;
        if (!player->master_params.parent.pause && !Player_has_stopped(player))
        {
            if (!player->cgiters_accessed)
            {
                // We are reading notes for the first time, do final inits
                player->cgiters_accessed = true;
                Player_init_final(player);
            }
            to_be_rendered = Player_move_forwards(player, to_be_rendered, false);
        }

        // Don't add padding audio if stopped during this call
        if (was_playing && Player_has_stopped(player))
        {
            rassert(to_be_rendered == 0);
            break;
        }

        // Process voices
        Player_process_voices(player, rendered, to_be_rendered);

        // Update carried controls
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            Channel* ch = player->channels[i];

            {
                Force_controls* fc = &ch->force_controls;

                if (Slider_in_progress(&fc->slider))
                    fc->force = (float)Slider_skip(&fc->slider, to_be_rendered);

                if (LFO_active(&fc->tremolo))
                    LFO_skip(&fc->tremolo, to_be_rendered);
            }

            {
                Pitch_controls* pc = &ch->pitch_controls;

                if (Slider_in_progress(&pc->slider))
                    pc->pitch = Slider_skip(&pc->slider, to_be_rendered);

                if (LFO_active(&pc->vibrato))
                    LFO_skip(&pc->vibrato, to_be_rendered);
            }

            Channel_stream_state_update(ch->csstate, to_be_rendered);
        }

        // Process signals in the connection graph
        {
            const int32_t buf_start = rendered;
            const int32_t buf_stop = rendered + to_be_rendered;

            Connections_process_mixed_signals(
                    connections,
                    true, // hack_reset
                    player->device_states,
                    player->work_buffers,
                    buf_start,
                    buf_stop,
                    player->audio_rate,
                    player->master_params.tempo);

            if (player->module->is_dc_blocker_enabled)
                Player_apply_dc_blocker(player, buf_start, buf_stop);

            Player_apply_master_volume(player, buf_start, buf_stop);
        }

        rendered += to_be_rendered;
    }

    // Apply global parameters to the mixed signal
    {
        Device_state* master_state = Device_states_get_state(
                player->device_states, Device_get_id((const Device*)player->module));
        rassert(master_state != NULL);

        // Note: we only access as many ports as we can output
        for (int32_t port = 0; port < KQT_BUFFERS_MAX; ++port)
        {
            float* out_buf = player->audio_buffers[port];

            Work_buffer* buffer = Device_state_get_audio_buffer(
                    master_state, DEVICE_PORT_TYPE_RECEIVE, port);

            if (buffer != NULL)
            {
                // Apply render volume
                const float* buf = Work_buffer_get_contents(buffer);

                const float mix_vol = (float)player->module->mix_vol;
                for (int32_t i = 0; i < rendered; ++i)
                    out_buf[i] = buf[i] * mix_vol;
            }
            else
            {
                // Fill with zeroes if we haven't produced any sound
                for (int32_t i = 0; i < rendered; ++i)
                    player->audio_buffers[port][i] = 0;
            }
        }
    }

    player->audio_frames_available = rendered;

    player->audio_frames_processed += rendered;

    player->events_returned = false;

    return;
}


void Player_skip(Player* player, int64_t nframes)
{
    rassert(player != NULL);
    rassert(nframes >= 0);

    // Clear buffers as we're not providing meaningful output
    Event_buffer_clear(player->event_buffer);
    player->audio_frames_available = 0;

    if (Player_has_stopped(player) || player->master_params.parent.pause)
        return;

    // TODO: check if song or pattern instance location has changed

    // Composition-level progress
    int64_t skipped = 0;
    while (skipped < nframes)
    {
        if (!player->cgiters_accessed)
        {
            // We are reading notes for the first time, do final inits
            player->cgiters_accessed = true;
            Player_init_final(player);
        }

        // Move forwards in composition
        int32_t to_be_skipped = (int32_t)min(nframes - skipped, INT32_MAX);
        to_be_skipped = Player_move_forwards(player, to_be_skipped, true);

        if (Player_has_stopped(player))
        {
            rassert(to_be_skipped == 0);
            break;
        }

        // Update master volume slider
        Slider_skip(&player->master_params.volume_slider, to_be_skipped);

        skipped += to_be_skipped;
    }

    player->audio_frames_processed += skipped;

    player->events_returned = false;

    if (nframes > 0)
        player->cgiters_accessed = true;

    return;
}


int32_t Player_get_frames_available(const Player* player)
{
    rassert(player != NULL);
    return player->audio_frames_available;
}


const float* Player_get_audio(const Player* player, int channel)
{
    rassert(player != NULL);
    rassert(channel == 0 || channel == 1);
    return player->audio_buffers[channel];
}


const char* Player_get_events(Player* player)
{
    rassert(player != NULL);

    if (player->events_returned)
    {
        // Get more events if row processing was interrupted
        Player_update_receive(player);
    }

    player->events_returned = true;

    return Event_buffer_get_events(player->event_buffer);
}


bool Player_has_stopped(const Player* player)
{
    rassert(player != NULL);
    return (player->master_params.playback_state == PLAYBACK_STOPPED);
}


bool Player_fire(Player* player, int ch, Streader* event_reader)
{
    rassert(player != NULL);
    rassert(ch >= 0);
    rassert(ch < KQT_CHANNELS_MAX);
    rassert(event_reader != NULL);

    if (Streader_is_error_set(event_reader))
        return false;

    Player_flush_receive(player);

    Event_buffer_clear(player->event_buffer);

    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[EVENT_NAME_MAX + 1] = "";
    Event_type type = Event_NONE;

    // Get event name
    if (!get_event_type_info(event_reader, event_names, event_name, &type))
        return false;

    // Get event argument
    Value* value = VALUE_AUTO;
    value->type = Event_names_get_param_type(event_names, event_name);

    switch (value->type)
    {
        case VALUE_TYPE_NONE:
            Streader_read_null(event_reader);
            break;

        case VALUE_TYPE_BOOL:
            Streader_read_bool(event_reader, &value->value.bool_type);
            break;

        case VALUE_TYPE_INT:
            Streader_read_int(event_reader, &value->value.int_type);
            break;

        case VALUE_TYPE_FLOAT:
            Streader_read_float(event_reader, &value->value.float_type);
            break;

        case VALUE_TYPE_TSTAMP:
            Streader_read_tstamp(event_reader, &value->value.Tstamp_type);
            break;

        case VALUE_TYPE_STRING:
            Streader_read_string(
                    event_reader, KQT_VAR_NAME_MAX, value->value.string_type);
            break;

        case VALUE_TYPE_PAT_INST_REF:
            Streader_read_piref(event_reader, &value->value.Pat_inst_ref_type);
            break;

        case VALUE_TYPE_REALTIME:
            Streader_read_finite_rt(event_reader, value);
            break;

        case VALUE_TYPE_MAYBE_STRING:
        {
            if (Streader_read_null(event_reader))
            {
                value->type = VALUE_TYPE_NONE;
            }
            else
            {
                value->type = VALUE_TYPE_STRING;
                Streader_clear_error(event_reader);
                Streader_read_string(
                        event_reader, KQT_VAR_NAME_MAX, value->value.string_type);
            }
        }
        break;

        default:
            rassert(false);
    }

    if (!Streader_match_char(event_reader, ']'))
        return false;

    // Fire
    Player_process_event(player, ch, event_name, value, false);

    // Check and perform goto if needed
    Player_check_perform_goto(player);

    // Store event parameters if processing was suspended
    if (Event_buffer_is_skipping(player->event_buffer))
    {
        player->susp_event_ch = ch;
        strcpy(player->susp_event_name, event_name);
        Value_copy(&player->susp_event_value, value);
    }
    else
    {
        Event_buffer_reset_add_counter(player->event_buffer);
    }

    player->events_returned = false;

    return true;
}


void del_Player(Player* player)
{
    if (player == NULL)
        return;

    del_Event_handler(player->event_handler);
    del_Voice_pool(player->voices);
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        del_Channel(player->channels[i]);
    Master_params_deinit(&player->master_params);
    del_Work_buffers(player->work_buffers);
    del_Event_buffer(player->event_buffer);
    del_Env_state(player->estate);
    del_Device_states(player->device_states);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        memory_free(player->audio_buffers[i]);

    memory_free(player);
    return;
}


