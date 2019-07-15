

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
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
#include <Error.h>
#include <init/devices/Au_params.h>
#include <init/devices/Audio_unit.h>
#include <init/sheet/Channel_defaults.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <memory.h>
#include <Pat_inst_ref.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Voice_state.h>
#include <player/Mixed_signal_plan.h>
#include <player/Player_private.h>
#include <player/Player_seq.h>
#include <player/Position.h>
#include <player/Tuning_state.h>
#include <player/Voice_group.h>
#include <player/Voice_group_reservations.h>
#include <player/Voice_signal_plan.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>
#include <string/common.h>
#include <threads/Barrier.h>
#include <threads/Condition.h>
#include <threads/Mutex.h>
#include <threads/Thread.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifdef ENABLE_THREADS
static void* render_thread_func(void* arg);
#endif


static bool Player_prepare_mixing_with_thread_count(Player* player, int thread_count);


static void Player_thread_params_init(
        Player_thread_params* tp, Player* player, int thread_id)
{
    rassert(tp != NULL);
    rassert(player != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);

    tp->player = player;
    tp->thread_id = thread_id;
    tp->active_voices = 0;
    tp->active_vgroups = 0;
    tp->work_buffers = NULL;
    for (int ch = 0; ch < 2; ++ch)
        tp->test_voice_outputs[ch] = NULL;

    return;
}


static void Player_thread_params_deinit(Player_thread_params* tp)
{
    rassert(tp != NULL);

    del_Work_buffers(tp->work_buffers);
    tp->work_buffers = NULL;
    for (int ch = 0; ch < 2; ++ch)
    {
        del_Work_buffer(tp->test_voice_outputs[ch]);
        tp->test_voice_outputs[ch] = NULL;
    }

    return;
}


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
    player->audio_buffer = NULL;
    player->audio_frames_available = 0;

    player->thread_count = 0;
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        Player_thread_params_init(&player->thread_params[i], player, i);
    player->start_cond = *CONDITION_AUTO;
    player->vgroups_start_barrier = *BARRIER_AUTO;
    player->vgroups_finished_barrier = *BARRIER_AUTO;
    player->mixed_start_barrier = *BARRIER_AUTO;
    player->mixed_level_finished_barrier = *BARRIER_AUTO;
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        player->threads[i] = *THREAD_AUTO;
    player->ok_to_start = false;
    player->early_exit_threads = false;
    player->stop_threads = false;
    player->render_frame_count = 0;

    player->device_states = NULL;
    player->estate = NULL;
    player->event_buffer = NULL;
    player->voices = NULL;
    player->mixed_signal_plan = NULL;
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
    memset(player->susp_event_name, '\0', KQT_EVENT_NAME_MAX + 1);
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

    if (Master_params_init(
                &player->master_params, player->module, player->estate) == NULL)
    {
        del_Player(player);
        return NULL;
    }

    Voice_group_reservations_init(&player->voice_group_res);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        player->channels[i] = new_Channel(
                player->module,
                i,
                Module_get_au_table(player->module),
                player->estate,
                player->voices,
                &player->voice_group_res,
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
        player->audio_buffer =
            memory_alloc_items(float, player->audio_buffer_size * KQT_BUFFERS_MAX);
        if (player->audio_buffer == NULL)
        {
            del_Player(player);
            return NULL;
        }
    }

    if (!Player_set_thread_count(player, 1, ERROR_AUTO))
    {
        del_Player(player);
        return NULL;
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


static bool Player_thread_params_create_buffers(
        Player_thread_params* tp, int32_t audio_buffer_size)
{
    rassert(tp != NULL);
    rassert(tp->work_buffers == NULL);
    rassert(audio_buffer_size >= 0);

    tp->work_buffers = new_Work_buffers(audio_buffer_size);
    if (tp->work_buffers == NULL)
    {
        Player_thread_params_deinit(tp);
        return false;
    }

    if (audio_buffer_size > 0)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            rassert(tp->test_voice_outputs[ch] == NULL);
            tp->test_voice_outputs[ch] = new_Work_buffer(audio_buffer_size);
            if (tp->test_voice_outputs[ch] == NULL)
            {
                Player_thread_params_deinit(tp);
                return false;
            }
        }
    }

    return true;
}


bool Player_set_thread_count(Player* player, int new_count, Error* error)
{
    rassert(player != NULL);
    rassert(new_count >= 1);
    rassert(new_count <= KQT_THREADS_MAX);
    rassert(error != NULL);

#ifndef ENABLE_THREADS
    // Override requested thread count if threads are not supported
    new_count = 1;
#endif

    if (Error_is_set(error))
        return false;

    if (new_count == player->thread_count)
        return true;

    const int old_count = player->thread_count;
    player->thread_count = min(old_count, new_count);

    // (De)allocate player Work buffers as needed
    for (int i = new_count; i < old_count; ++i)
    {
        del_Work_buffers(player->thread_params[i].work_buffers);
        player->thread_params[i].work_buffers = NULL;
    }
    for (int i = old_count; i < new_count; ++i)
    {
        if (!Player_thread_params_create_buffers(
                    &player->thread_params[i], player->audio_buffer_size))
        {
            Error_set(
                    error,
                    ERROR_MEMORY,
                    "Could not allocate memory for new work buffers");
            return false;
        }
    }

    // (De)allocate Work buffers of Device states as needed
    if (!Device_states_set_thread_count(player->device_states, new_count) ||
            !Player_prepare_mixing_with_thread_count(player, new_count))
    {
        Error_set(
                error,
                ERROR_MEMORY,
                "Could not allocate memory for new device states");
        return false;
    }

#ifdef ENABLE_THREADS

    const int threads_needed = (new_count > 1) ? new_count : 0;

    // Remove old threads (all of them so that we can replace our barriers)
    if (old_count > 1)
    {
        player->stop_threads = true;
        Barrier_wait(&player->vgroups_start_barrier);
        for (int i = 0; i < KQT_THREADS_MAX; ++i)
        {
            if (!Thread_is_initialised(&player->threads[i]))
                continue;

            Thread_join(&player->threads[i]);
        }
        player->stop_threads = false;
    }

    // Deinitialise old barriers
    Barrier_deinit(&player->vgroups_start_barrier);
    Barrier_deinit(&player->vgroups_finished_barrier);
    Barrier_deinit(&player->mixed_start_barrier);
    Barrier_deinit(&player->mixed_level_finished_barrier);

    // Create new barriers
    if (threads_needed > 0)
    {
        const int count = threads_needed + 1;

        if (!Barrier_init(&player->vgroups_start_barrier, count, error) ||
                !Barrier_init(&player->vgroups_finished_barrier, count, error) ||
                !Barrier_init(&player->mixed_start_barrier, count, error) ||
                !Barrier_init(&player->mixed_level_finished_barrier, count, error))
            return false;
    }

    if ((threads_needed > 0) && !Condition_is_initialised(&player->start_cond))
        Condition_init(&player->start_cond);

    player->ok_to_start = false;

    // Create new threads
    for (int i = 0; i < threads_needed; ++i)
    {
        if (!Thread_init(
                    &player->threads[i],
                    render_thread_func,
                    &player->thread_params[i],
                    error))
        {
            // We roll back here because dealing with this special case
            // in the destructor would get messy
            Mutex* mutex = Condition_get_mutex(&player->start_cond);
            Mutex_lock(mutex);
            player->early_exit_threads = true;
            player->ok_to_start = true;
            Condition_broadcast(&player->start_cond);
            Mutex_unlock(mutex);

            for (int k = i - 1; k >= 0; --k)
                Thread_join(&player->threads[k]);

            player->early_exit_threads = false;

            player->thread_count = 1;

            return false;
        }
    }

    if (threads_needed > 0)
    {
        Mutex* mutex = Condition_get_mutex(&player->start_cond);
        Mutex_lock(mutex);
        player->ok_to_start = true;
        Condition_broadcast(&player->start_cond);
        Mutex_unlock(mutex);
    }
    else
    {
        player->ok_to_start = true;
    }

#endif

    player->thread_count = new_count;

    return true;
}


int Player_get_thread_count(const Player* player)
{
    rassert(player != NULL);
    return player->thread_count;
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


static bool Player_prepare_mixing_with_thread_count(Player* player, int thread_count)
{
    rassert(player != NULL);
    rassert(thread_count > 0);
    rassert(thread_count <= KQT_THREADS_MAX);

    del_Mixed_signal_plan(player->mixed_signal_plan);
    player->mixed_signal_plan = NULL;

    const Connections* conns = Module_get_connections(player->module);
    if (conns == NULL)
        return true;

    if (!Device_states_prepare(player->device_states, conns))
        return false;

    {
        Au_table* au_table = Module_get_au_table(player->module);
        for (int i = 0; i < KQT_AUDIO_UNITS_MAX; ++i)
        {
            const Audio_unit* au = Au_table_get(au_table, i);
            if ((au != NULL) &&
                    Device_is_existent((const Device*)au) &&
                    (Audio_unit_get_type(au) == AU_TYPE_INSTRUMENT))
            {
                const Connections* au_conns = Audio_unit_get_connections(au);
                if (au_conns != NULL)
                {
                    const uint32_t au_id = Device_get_id((const Device*)au);
                    Au_state* au_state =
                        (Au_state*)Device_states_get_state(player->device_states, au_id);

                    Voice_signal_plan* plan = new_Voice_signal_plan(
                            player->device_states, thread_count, au_conns);
                    if (plan == NULL)
                        return false;
                    Au_state_set_voice_signal_plan(au_state, plan);
                }
            }
        }
    }

    player->mixed_signal_plan = new_Mixed_signal_plan(player->device_states, conns);
    if (player->mixed_signal_plan == NULL)
        return false;

    return true;
}


bool Player_prepare_mixing(Player* player)
{
    rassert(player != NULL);
    return Player_prepare_mixing_with_thread_count(player, player->thread_count);
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
    if (track_num == -1)
    {
        player->master_params.playback_state = PLAYBACK_MODULE;
        player->master_params.cur_pos.track = 0;
    }
    else
    {
        player->master_params.cur_pos.track = track_num;
    }

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


static bool Player_thread_params_set_audio_buffer_size(
        Player_thread_params* tp, int32_t audio_buffer_size)
{
    rassert(tp != NULL);

    if ((tp->work_buffers != NULL) &&
            !Work_buffers_resize(tp->work_buffers, audio_buffer_size))
        return false;

    for (int ch = 0; ch < 2; ++ch)
    {
        if ((tp->test_voice_outputs[ch] != NULL) &&
                !Work_buffer_resize(tp->test_voice_outputs[ch], audio_buffer_size))
            return false;
    }

    return true;
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
        memory_free(player->audio_buffer);
    }
    else
    {
        float* new_buffer =
            memory_realloc_items(float, size * KQT_BUFFERS_MAX, player->audio_buffer);
        if (new_buffer == NULL)
            return false;

        player->audio_buffer = new_buffer;
    }

    // Update device state buffers
    if (!Device_states_set_audio_buffer_size(player->device_states, size))
        return false;

    // Update work buffers
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
    {
        if (!Player_thread_params_set_audio_buffer_size(&player->thread_params[i], size))
            return false;
    }

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


typedef struct Render_stats
{
    int voice_count;
    int vgroup_count;
} Render_stats;

#define RENDER_STATS_AUTO (&(Render_stats){ .voice_count = 0, .vgroup_count = 0 })


static void Player_process_voice_group(
        Player* player,
        Player_thread_params* tparams,
        Voice_group* vgroup,
        int32_t frame_count,
        int32_t frame_offset,
        int32_t total_frame_count,
        Render_stats* stats)
{
    rassert(player != NULL);
    rassert(tparams != NULL);
    rassert(vgroup != NULL);
    rassert(frame_count >= 0);
    rassert(frame_offset >= 0);
    rassert(total_frame_count >= frame_count);
    rassert(stats != NULL);

    // Find the connections that contain the processors
    const Voice* first_voice = Voice_group_get_voice(vgroup, 0);
    const Processor* first_proc = Voice_get_proc(first_voice);
    const Au_params* first_au_params = Processor_get_au_params(first_proc);
    const uint32_t au_id = first_au_params->device_id;
    Au_state* au_state =
        (Au_state*)Device_states_get_state(player->device_states, au_id);
    //const Connections* conns = Audio_unit_get_connections(au);

    const bool use_test_output = Voice_is_using_test_output(first_voice);
    int32_t test_output_stop = frame_count;

    Voice_signal_plan* plan = au_state->voice_signal_plan;

    if (plan != NULL)
    {
        const int ch_num = Voice_group_get_ch_num(vgroup);
        const bool is_muted = !Voice_group_is_external(vgroup)
            ? Channel_is_muted(player->channels[ch_num]) : false;
        const bool enable_mixing = !is_muted && !use_test_output;

        const int32_t process_stop = Voice_signal_plan_execute(
                plan,
                player->device_states,
                tparams->thread_id,
                vgroup,
                tparams->work_buffers,
                frame_count,
                frame_offset,
                total_frame_count,
                player->master_params.tempo,
                enable_mixing);

        test_output_stop = process_stop;

        if (process_stop < frame_count)
            Voice_group_deactivate_all(vgroup);
        //else
        //    Voice_group_deactivate_unreachable(vgroup);

        const int active_voice_count = Voice_group_get_active_count(vgroup);
        stats->voice_count += active_voice_count;
        if (active_voice_count > 0)
            ++stats->vgroup_count;
    }
    else
    {
        Voice_group_deactivate_all(vgroup);
    }

    if (use_test_output)
    {
        // Mix output signal of our test processor
        const Audio_unit* au =
            (const Audio_unit*)Device_state_get_device((const Device_state*)au_state);
        const Processor* test_proc =
            Audio_unit_get_proc(au, Voice_get_test_proc_index(first_voice));
        const uint32_t test_proc_id = Device_get_id((const Device*)test_proc);
        Device_thread_state* test_ts = Device_states_get_thread_state(
                player->device_states, tparams->thread_id, test_proc_id);

        // Don't feel like figuring out a generic solution right now, TODO

        const Work_buffer* in_wb =
            Device_thread_state_get_voice_buffer(test_ts, DEVICE_PORT_TYPE_SEND, 0);
        if (Work_buffer_is_valid(in_wb))
        {
            const Work_buffer* in_wbs[] =
            {
                in_wb,
                Device_thread_state_get_voice_buffer(test_ts, DEVICE_PORT_TYPE_SEND, 1),
            };
            if (!Work_buffer_is_valid(in_wbs[1]))
                in_wbs[1] = in_wbs[0];

            for (int si = 0; si < 2; ++si)
            {
                Work_buffer_mix(
                        tparams->test_voice_outputs[si],
                        in_wbs[si],
                        0,
                        test_output_stop);

                Work_buffer_clear(
                        tparams->test_voice_outputs[si], test_output_stop, frame_count);
            }
        }
    }

    return;
}


static void Player_process_channel_fg_voices(
        Player* player,
        Player_thread_params* tparams,
        int ch_index,
        int32_t frame_count,
        Render_stats* stats)
{
    rassert(player != NULL);
    rassert(tparams != NULL);
    rassert(ch_index >= 0);
    rassert(ch_index < KQT_CHANNELS_MAX);
    rassert(frame_count > 0);
    rassert(stats != NULL);

    Channel* ch = player->channels[ch_index];
    ch->fg_group_temp = *VOICE_GROUP_AUTO;

    // Find our current foreground voice
    {
        Voice_group* vgroup = VOICE_GROUP_AUTO;

        int ch_iter = 0;
        Voice_pool_start_fg_ch_iteration(player->voices, ch_index, &ch_iter);

        Voice_group* vg =
            Voice_pool_get_next_fg_group(player->voices, ch_index, &ch_iter, vgroup);
        while (vg != NULL)
        {
            const Voice* first_voice = Voice_group_get_voice(vg, 0);
            rassert(first_voice->ch_num == ch_index);

            if (ch->fg_group_id == first_voice->group_id)
            {
                // Use the currently active Voice group
                rassert(ch->fg_group_temp.size == 0);
                Voice_group_copy(&ch->fg_group_temp, vg);

                break;
            }

            vg = Voice_pool_get_next_fg_group(
                    player->voices, ch_index, &ch_iter, vgroup);
        }
    }

    const Channel_event_buffer* events = &ch->local_events;
    const int event_count = Channel_event_buffer_get_event_count(events);

    int event_index = 0;
    int32_t slice_start = 0;

    while (slice_start < frame_count)
    {
        int32_t slice_stop = frame_count;

        const Channel_event* event = NULL;
        if (event_index < event_count)
        {
            event = Channel_event_buffer_get_event(events, event_index);
            rassert(event->frame_offset <= frame_count);
            slice_stop = event->frame_offset;

            ++event_index;
        }

        if (slice_start < slice_stop)
        {
            const int32_t cur_frame_count = slice_stop - slice_start;

            if (ch->fg_group_id != 0)
            {
                Voice_group* vg = &ch->fg_group_temp;
                if (Voice_group_get_size(vg) != 0)
                {
                    //fprintf(stderr, "process foreground voice group %d, frames [%d, %d)\n",
                    //        (int)Voice_group_get_voice(vg, 0)->group_id,
                    //        (int)slice_start, (int)slice_stop);
                    Player_process_voice_group(
                            player,
                            tparams,
                            vg,
                            cur_frame_count,
                            slice_start,
                            frame_count,
                            stats);
                }
            }

            // Update carried controls
            {
                Force_controls* fc = &ch->force_controls;

                if (Slider_in_progress(&fc->slider))
                    fc->force = (float)Slider_skip(&fc->slider, cur_frame_count);

                if (LFO_active(&fc->tremolo))
                    LFO_skip(&fc->tremolo, cur_frame_count);
            }

            {
                Pitch_controls* pc = &ch->pitch_controls;

                if (Slider_in_progress(&pc->slider))
                    pc->pitch = Slider_skip(&pc->slider, cur_frame_count);

                if (LFO_active(&pc->vibrato))
                    LFO_skip(&pc->vibrato, cur_frame_count);
            }

            Channel_stream_state_update(ch->csstate, cur_frame_count);
        }

        if (event != NULL)
        {
            ch->frame_offset_temp = event->frame_offset;

            const uint64_t prev_fg_group_id = ch->fg_group_id;

            /*
            {
                fprintf(stdout, "processing local event %d", (int)event->type);
                if (event->argument.type != VALUE_TYPE_NONE)
                {
                    char arg_s[128] = "";
                    Value_serialise(&event->argument, 128, arg_s);
                    fprintf(stdout, " %s", arg_s);
                }
                fprintf(stdout, ", frame offset %d\n", event->frame_offset);
            }
            */

            const bool external = false;
            Event_handler_trigger_by_type(
                    player->event_handler,
                    ch_index,
                    event->type,
                    &event->argument,
                    external);

            if (prev_fg_group_id != ch->fg_group_id)
            {
                if (ch->fg_group_id != 0)
                {
                    Voice_group* new_group = Voice_pool_get_fg_group(
                            player->voices,
                            ch_index,
                            ch->fg_group_id,
                            VOICE_GROUP_AUTO);
                    if (new_group != NULL)
                        Voice_group_copy(&ch->fg_group_temp, new_group);
                    else
                        ch->fg_group_temp = *VOICE_GROUP_AUTO;
                }
                else
                {
                    ch->fg_group_temp = *VOICE_GROUP_AUTO;
                }
            }

            ch->frame_offset_temp = 0;
        }

        slice_start = slice_stop;
    }

    for (; event_index < event_count; ++event_index)
    {
        const Channel_event* event =
            Channel_event_buffer_get_event(events, event_index);
        rassert(event->frame_offset == frame_count);

        const bool external = false;
        Event_handler_trigger_by_type(
                player->event_handler,
                ch_index,
                event->type,
                &event->argument,
                external);
    }

    // Process voices that were moved to background
    {
        Voice_group* vgroup = VOICE_GROUP_AUTO;

        int ch_iter = 0;
        Voice_pool_start_fg_ch_iteration(player->voices, ch_index, &ch_iter);

        Voice_group* vg =
            Voice_pool_get_next_fg_group(player->voices, ch_index, &ch_iter, vgroup);
        while (vg != NULL)
        {
            if (Voice_group_is_bg(vg))
            {
                Voice* first_voice = Voice_group_get_voice(vg, 0);
                const int32_t frame_offset = first_voice->frame_offset;
                const int32_t cur_frame_count = frame_count - frame_offset;

                //fprintf(stderr, "process new background voice group %d, frames [%d, %d)\n",
                //        (int)Voice_group_get_voice(vg, 0)->group_id,
                //        (int)frame_offset, (int)frame_count);
                Player_process_voice_group(
                        player,
                        tparams,
                        vg,
                        cur_frame_count,
                        frame_offset,
                        frame_count,
                        stats);

                for (int vi = 0; vi < Voice_group_get_size(vg); ++vi)
                    Voice_group_get_voice(vg, vi)->frame_offset = 0;
            }

            vg = Voice_pool_get_next_fg_group(
                    player->voices, ch_index, &ch_iter, vgroup);
        }
    }

    Channel_event_buffer_init(&ch->local_events);

    return;
}


#ifdef ENABLE_THREADS
static void Player_process_voice_groups_synced(
        Player* player, Player_thread_params* tparams, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(tparams != NULL);
    rassert(frame_count >= 0);

    Render_stats* stats = RENDER_STATS_AUTO;

    // Foreground voices assigned to this thread
    const int ch_skip = player->thread_count;
    for (int ci = tparams->thread_id; ci < KQT_CHANNELS_MAX; ci += ch_skip)
        Player_process_channel_fg_voices(player, tparams, ci, frame_count, stats);

    // Background voices
    {
        Voice_group* vgroup = VOICE_GROUP_AUTO;

        Voice_group* vg = Voice_pool_get_next_bg_group_synced(player->voices, vgroup);
        while (vg != NULL)
        {
            Voice* first_voice = Voice_group_get_voice(vg, 0);
            const int32_t frame_offset = first_voice->frame_offset;
            const int32_t cur_frame_count = frame_count - frame_offset;

            Player_process_voice_group(
                    player,
                    tparams,
                    vg,
                    cur_frame_count,
                    frame_offset,
                    frame_count,
                    stats);

            for (int vi = 0; vi < Voice_group_get_size(vg); ++vi)
                Voice_group_get_voice(vg, vi)->frame_offset = 0;

            vg = Voice_pool_get_next_bg_group_synced(player->voices, vgroup);
        }
    }

#if 0
    Voice_group* vgroup = VOICE_GROUP_AUTO;

    Voice_group* vg = Voice_pool_get_next_group_synced(player->voices, vgroup);
    while (vg != NULL)
    {
        Player_process_voice_group(player, tparams, vg, frame_count, stats);

        vg = Voice_pool_get_next_group_synced(player->voices, vgroup);
    }
#endif

    tparams->active_voices = stats->voice_count;
    tparams->active_vgroups = stats->vgroup_count;

    return;
}


#if 0
static void Player_execute_mixed_signal_tasks_synced(
        Player* player, Player_thread_params* tparams, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(tparams != NULL);
    rassert(frame_count > 0);

    const int level_count = Mixed_signal_plan_get_level_count(player->mixed_signal_plan);

    for (int level_index = level_count - 1; level_index >= 0; --level_index)
    {
        while (Mixed_signal_plan_execute_next_task(
                player->mixed_signal_plan,
                level_index,
                tparams->work_buffers,
                frame_count,
                player->master_params.tempo))
            ;

        Barrier_wait(&player->mixed_level_finished_barrier);
    }

    return;
}
#endif


static void* render_thread_func(void* arg)
{
    rassert(arg != NULL);

    Player_thread_params* params = arg;
    Player* player = params->player;

    // Wait for the initial starting call
    {
        Mutex* cond_mutex = Condition_get_mutex(&player->start_cond);
        Mutex_lock(cond_mutex);
        while (!player->ok_to_start)
            Condition_wait(&player->start_cond);
        Mutex_unlock(cond_mutex);
    }

    if (player->early_exit_threads)
        return NULL;

    while (true)
    {
        // Wait for our signal to start voice group processing
        Barrier_wait(&player->vgroups_start_barrier);

        if (player->stop_threads)
            break;

        rassert(params->thread_id < player->thread_count);

        Player_process_voice_groups_synced(player, params, player->render_frame_count);

        // Wait to indicate that we have finished processing voice groups
        Barrier_wait(&player->vgroups_finished_barrier);

        // Wait for our signal to start mixed signal processing
        //Barrier_wait(&player->mixed_start_barrier);

        //Player_execute_mixed_signal_tasks_synced(
        //        player, params, player->render_frame_count);
    }

    return NULL;
}
#endif


static void Player_process_all_local_events(Player* player)
{
    rassert(player != NULL);

    for (int ci = 0; ci < KQT_CHANNELS_MAX; ++ci)
    {
        Channel* ch = player->channels[ci];
        const Channel_event_buffer* events = &ch->local_events;
        const int event_count = Channel_event_buffer_get_event_count(events);

        for (int ei = 0; ei < event_count; ++ei)
        {
            const Channel_event* event = Channel_event_buffer_get_event(events, ei);
            rassert(event->frame_offset == 0);

            const bool external = false;
            Event_handler_trigger_by_type(
                    player->event_handler,
                    ci,
                    event->type,
                    &event->argument,
                    external);
        }

        Channel_event_buffer_init(&ch->local_events);
    }

    Voice_pool_clean_up_fg_voices(player->voices);

    return;
}


static void Player_process_voices_single_threaded(
        Player* player, int32_t frame_count, Render_stats* stats)
{
    rassert(player != NULL);
    rassert(frame_count > 0);
    rassert(stats != NULL);

    Voice_pool_start_group_iteration(player->voices);

    // Foreground voices
    for (int ci = 0; ci < KQT_CHANNELS_MAX; ++ci)
        Player_process_channel_fg_voices(
                player, &player->thread_params[0], ci, frame_count, stats);

    // Background voices
    {
        Voice_group* vgroup = VOICE_GROUP_AUTO;

        Voice_group* vg = Voice_pool_get_next_bg_group(player->voices, vgroup);
        while (vg != NULL)
        {
            Voice* first_voice = Voice_group_get_voice(vg, 0);
            const int32_t frame_offset = first_voice->frame_offset;
            const int32_t cur_frame_count = frame_count - frame_offset;

            //fprintf(stdout, "process background voice group %d, frames [%d, %d)\n",
            //        (int)Voice_group_get_voice(vg, 0)->group_id,
            //        (int)frame_offset, (int)frame_count);

            Player_process_voice_group(
                    player,
                    &player->thread_params[0],
                    vg,
                    cur_frame_count,
                    frame_offset,
                    frame_count,
                    stats);

            for (int vi = 0; vi < Voice_group_get_size(vg); ++vi)
                Voice_group_get_voice(vg, vi)->frame_offset = 0;

            vg = Voice_pool_get_next_bg_group(player->voices, vgroup);
        }
    }

    Voice_pool_finish_group_iteration(player->voices);

    //fprintf(stdout, "---\n");

    return;
}


static void Player_process_voices(Player* player, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(frame_count >= 0);

    if (frame_count == 0)
    {
        Player_process_all_local_events(player);
        return;
    }

    // Process active Voice groups
    int active_voice_count = 0;
    int active_vgroup_count = 0;

#ifdef ENABLE_THREADS
    if (player->thread_count > 1)
    {
        Voice_pool_start_group_iteration(player->voices);

        // Pass render start and stop parameters to threads
        player->render_frame_count = frame_count;

        // Synchronise with all threads to start voice group processing
        Barrier_wait(&player->vgroups_start_barrier);

        // Wait until all threads have finished
        Barrier_wait(&player->vgroups_finished_barrier);

        Voice_pool_finish_group_iteration(player->voices);

        // Calculate active voices
        for (int i = 0; i < player->thread_count; ++i)
        {
            active_voice_count += player->thread_params[i].active_voices;
            active_vgroup_count += player->thread_params[i].active_vgroups;
        }
    }
    else
#endif
    {
        // Process all voice groups in a single thread
        Render_stats* stats = RENDER_STATS_AUTO;

        Player_process_voices_single_threaded(player, frame_count, stats);

        active_voice_count = stats->voice_count;
        active_vgroup_count = stats->vgroup_count;
    }

    if (player->thread_count > 1)
        Device_states_mix_thread_states(player->device_states, 0, frame_count);

    player->master_params.active_voices =
        max(player->master_params.active_voices, active_voice_count);
    player->master_params.active_vgroups =
        max(player->master_params.active_vgroups, active_vgroup_count);

    return;
}


static void Player_process_mixed_signals(Player* player, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(frame_count >= 0);

    if (frame_count == 0)
        return;

    rassert(player->mixed_signal_plan != NULL);
#if 0
#ifdef ENABLE_THREADS
    if (player->thread_count > 1)
    {
        player->render_frame_count = frame_count;

        // Synchronise with all threads to start mixed task execution
        Barrier_wait(&player->mixed_start_barrier);

        if (frame_count > 0)
        {
            const int level_count =
                Mixed_signal_plan_get_level_count(player->mixed_signal_plan);
            for (int level_i = level_count - 1; level_i >= 0; --level_i)
            {
                // Wait for each level to be finished
                Barrier_wait(&player->mixed_level_finished_barrier);
            }

            Mixed_signal_plan_reset(player->mixed_signal_plan);
        }
    }
    else
#endif
#endif
    {
        if (frame_count > 0)
        {
            Mixed_signal_plan_execute_all_tasks(
                    player->mixed_signal_plan,
                    player->thread_params[0].work_buffers,
                    frame_count,
                    player->master_params.tempo);
        }
    }

    // Fill invalid buffer areas with silence
    {
        Device_thread_state* master_ts = Device_states_get_thread_state(
                player->device_states, 0, Device_get_id((const Device*)player->module));
        rassert(master_ts != NULL);

        for (int ch = 0; ch < KQT_BUFFERS_MAX; ++ch)
        {
            Work_buffer* master_wb = Device_thread_state_get_mixed_buffer(
                    master_ts, DEVICE_PORT_TYPE_RECV, ch);
            if (master_wb != NULL)
            {
                if (!Work_buffer_is_valid(master_wb))
                    Work_buffer_clear(master_wb, 0, frame_count);
            }
        }
    }

    return;
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
            const bool is_at_global_breakpoint = true;
            const int32_t frame_offset = 0;
            const bool skip = false;
            const bool external = false;
            Player_process_event(
                    player,
                    player->susp_event_ch,
                    player->susp_event_name,
                    &player->susp_event_value,
                    is_at_global_breakpoint,
                    frame_offset,
                    skip,
                    external);

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


static void Player_mix_test_voice_signals(Player* player, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(frame_count >= 0);

    if (frame_count == 0)
        return;

    Device_thread_state* master_ts = Device_states_get_thread_state(
            player->device_states, 0, Device_get_id((const Device*)player->module));
    rassert(master_ts != NULL);

    for (int thread_id = 0; thread_id < KQT_THREADS_MAX; ++thread_id)
    {
        Player_thread_params* tp = &player->thread_params[thread_id];

        for (int ch = 0; ch < 2; ++ch)
        {
            Work_buffer* master_wb = Device_thread_state_get_mixed_buffer(
                    master_ts, DEVICE_PORT_TYPE_RECV, ch);
            if (master_wb == NULL)
                continue;

            const Work_buffer* test_wb = tp->test_voice_outputs[ch];
            if (!Work_buffer_is_valid(test_wb))
                continue;

            float first_val = Work_buffer_get_contents(test_wb)[0];

            if ((Work_buffer_get_const_start(test_wb) > 0) || (first_val != 0))
                Work_buffer_mix(master_wb, test_wb, 0, frame_count);
        }
    }

    return;
}


static void Player_apply_dc_blocker(Player* player, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(frame_count >= 0);

    if (frame_count == 0)
        return;

    // Get access to mixed output
    Device_thread_state* master_ts = Device_states_get_thread_state(
            player->device_states, 0, Device_get_id((const Device*)player->module));
    rassert(master_ts != NULL);

    // Implementation based on https://ccrma.stanford.edu/~jos/filters/DC_Blocker.html
    static const double adapt_time = 0.01;
    const double adapt_time_frames = max(2, adapt_time * player->audio_rate);
    const float R = (float)((adapt_time_frames - 1) / adapt_time_frames);
    const float gain = (1 + R) / 2;

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* wb = Device_thread_state_get_mixed_buffer(
                master_ts, DEVICE_PORT_TYPE_RECV, ch);
        if (wb == NULL)
            continue;

        float feedforward = player->master_params.dc_block_state[ch].feedforward;
        float feedback = player->master_params.dc_block_state[ch].feedback;

        float* buf = Work_buffer_get_contents_mut(wb);
        for (int32_t i = 0; i < frame_count; ++i)
        {
            const float in = *buf;

            const float out = gain * (in - feedforward) + R * feedback;
            *buf++ = out;
            feedforward = in;
            feedback = out;
        }

        player->master_params.dc_block_state[ch].feedforward = feedforward;
        player->master_params.dc_block_state[ch].feedback = feedback;
    }

    return;
}


static void Player_apply_master_volume(Player* player, int32_t frame_count)
{
    rassert(player != NULL);
    rassert(frame_count >= 0);

    Device_thread_state* master_ts = Device_states_get_thread_state(
            player->device_states, 0, Device_get_id((const Device*)player->module));
    rassert(master_ts != NULL);

    Work_buffer* master_wbs[2] = { NULL };
    for (int ch = 0; ch < 2; ++ch)
        master_wbs[ch] = Device_thread_state_get_mixed_buffer(
                master_ts, DEVICE_PORT_TYPE_RECV, ch);

    if (!Work_buffer_is_valid(master_wbs[0]) && !Work_buffer_is_valid(master_wbs[1]))
    {
        Slider_skip(&player->master_params.volume_slider, frame_count);
        return;
    }

    Slider slider;
    Slider_init(&slider, SLIDE_MODE_LINEAR);

    for (int ch = 0; ch < 2; ++ch)
    {
        Work_buffer* wb = master_wbs[ch];
        if (!Work_buffer_is_valid(wb))
            continue;

        Slider_copy(&slider, &player->master_params.volume_slider);

        float* buf = Work_buffer_get_contents_mut(wb);

        if (Slider_in_progress(&slider))
        {
            double final_volume = player->master_params.volume;
            for (int32_t i = 0; i < frame_count; ++i)
            {
                final_volume = Slider_step(&slider);
                *buf++ *= (float)final_volume;
            }
            player->master_params.volume = final_volume;
        }
        else
        {
            const float cur_volume = (float)player->master_params.volume;
            for (int32_t i = 0; i < frame_count; ++i)
                *buf++ *= cur_volume;
        }
    }

    Slider_copy(&player->master_params.volume_slider, &slider);

    return;
}


static void Player_init_final(Player* player)
{
    rassert(player != NULL);

    Master_params_set_starting_tempo(&player->master_params);

    Device_states_set_tempo(player->device_states, player->master_params.tempo);

    Player_reset_channels(player);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel* ch = player->channels[i];
        Channel_set_tempo(ch, player->master_params.tempo);
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);

    return;
}


static clock_t voice_time = 0;
static clock_t mixed_time = 0;
static clock_t total_time = 0;

static clock_t total_time_at_voice_max = 0;
static clock_t voice_time_max = 0;


void Player_play(Player* player, int32_t nframes)
{
    clock_t cur_voice_time = 0;
    const clock_t total_start_time = clock();

    rassert(player != NULL);
    rassert(player->audio_buffer_size > 0);
    rassert(nframes >= 0);

    Player_flush_receive(player);

    Event_buffer_clear(player->event_buffer);

    nframes = min(nframes, player->audio_buffer_size);

    const Connections* connections = Module_get_connections(player->module);
    rassert(connections != NULL);
    rassert(player->mixed_signal_plan != NULL);

    // TODO: check if song or pattern instance location has changed

    // Composition-level progress
    const bool was_playing = !Player_has_stopped(player);
    int32_t rendered = 0;
    while (rendered < nframes && !Event_buffer_is_full(player->event_buffer))
    {
        for (int ci = 0; ci < KQT_CHANNELS_MAX; ++ci)
            Channel_event_buffer_init(&player->channels[ci]->local_events);

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
            nframes = rendered + to_be_rendered;

        // Invalidate mixed signal buffers
        Device_states_invalidate_mixed_buffers(player->device_states);
        for (int thread_id = 0; thread_id < player->thread_count; ++thread_id)
        {
            Player_thread_params* tp = &player->thread_params[thread_id];
            for (int ch = 0; ch < 2; ++ch)
                Work_buffer_invalidate(tp->test_voice_outputs[ch]);
        }

        // Process voices
        const clock_t voice_start_time = clock();
        Player_process_voices(player, to_be_rendered);
        const clock_t voice_end_time = clock();
        rassert(voice_start_time <= voice_end_time);
        const clock_t this_voice_time = (voice_end_time - voice_start_time);
        cur_voice_time += this_voice_time;
        voice_time += this_voice_time;

        if (!Event_buffer_is_skipping(player->event_buffer))
            Voice_group_reservations_init(&player->voice_group_res);

        // Process mixed signals in the connection graph
        {
            const clock_t mixed_start_time = clock();
            Player_process_mixed_signals(player, to_be_rendered);
            const clock_t mixed_end_time = clock();
            rassert(mixed_start_time <= mixed_end_time);
            mixed_time += (mixed_end_time - mixed_start_time);

            Player_apply_master_volume(player, to_be_rendered);

            Player_mix_test_voice_signals(player, to_be_rendered);

            if (player->module->is_dc_blocker_enabled)
                Player_apply_dc_blocker(player, to_be_rendered);

            // Apply global parameters to the mixed signal
            {
                Device_thread_state* master_ts = Device_states_get_thread_state(
                        player->device_states,
                        0,
                        Device_get_id((const Device*)player->module));
                rassert(master_ts != NULL);

                static const int MASTER_WB_SILENT_OUT = 0;
                Work_buffer* silent_wb = Work_buffers_get_buffer_mut(
                        player->thread_params[0].work_buffers, MASTER_WB_SILENT_OUT);
                bool silent_cleared = false;

                const Work_buffer* master_wbs[KQT_BUFFERS_MAX] = { NULL };
                for (int ch = 0; ch < KQT_BUFFERS_MAX; ++ch)
                {
                    master_wbs[ch] = Device_thread_state_get_mixed_buffer(
                            master_ts, DEVICE_PORT_TYPE_RECV, ch);
                    if (!Work_buffer_is_valid(master_wbs[ch]))
                    {
                        if (!silent_cleared)
                        {
                            Work_buffer_clear(silent_wb, 0, to_be_rendered);
                            silent_cleared = true;
                        }
                        master_wbs[ch] = silent_wb;
                    }
                }

                const float* master_in[KQT_BUFFERS_MAX] = { NULL };
                for (int ch = 0; ch < KQT_BUFFERS_MAX; ++ch)
                    master_in[ch] = Work_buffer_get_contents(master_wbs[ch]);

                // Apply render volume and copy to output
                const float mix_vol = (float)player->module->mix_vol;
                float* out = player->audio_buffer + (rendered * KQT_BUFFERS_MAX);
                for (int32_t i = 0; i < to_be_rendered; ++i)
                {
                    *out++ = *(master_in[0])++ * mix_vol;
                    *out++ = *(master_in[1])++ * mix_vol;
                }
            }
        }

        rendered += to_be_rendered;
    }

    player->audio_frames_available = rendered;

    player->audio_frames_processed += rendered;

    player->events_returned = false;

    const clock_t total_end_time = clock();
    rassert(total_start_time <= total_end_time);
    total_time += (total_end_time - total_start_time);

    if (cur_voice_time > voice_time_max)
    {
        voice_time_max = cur_voice_time;
        total_time_at_voice_max = total_end_time - total_start_time;
    }

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

        for (int ci = 0; ci < KQT_CHANNELS_MAX; ++ci)
            Channel_event_buffer_init(&player->channels[ci]->local_events);

        if (Player_has_stopped(player))
            nframes = skipped + to_be_skipped;

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


const float* Player_get_audio(const Player* player)
{
    rassert(player != NULL);
    return player->audio_buffer;
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


void Player_set_channel_mute(Player* player, int ch, bool mute)
{
    rassert(player != NULL);
    rassert(ch >= 0);
    rassert(ch < KQT_CHANNELS_MAX);

    Channel_set_muted(player->channels[ch], mute);

    return;
}


bool Player_fire(Player* player, int ch_num, Streader* event_reader)
{
    rassert(player != NULL);
    rassert(ch_num >= 0);
    rassert(ch_num < KQT_CHANNELS_MAX);
    rassert(event_reader != NULL);

    if (Streader_is_error_set(event_reader))
        return false;

    Player_flush_receive(player);

    Event_buffer_clear(player->event_buffer);

    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[KQT_EVENT_NAME_MAX + 1] = "";
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
                    event_reader, KQT_VAR_NAME_MAX + 1, value->value.string_type);
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
                        event_reader, KQT_VAR_NAME_MAX + 1, value->value.string_type);
            }
        }
        break;

        case VALUE_TYPE_MAYBE_REALTIME:
        {
            if (Streader_read_null(event_reader))
            {
                value->type = VALUE_TYPE_NONE;
            }
            else
            {
                value->type = VALUE_TYPE_STRING;
                Streader_clear_error(event_reader);
                Streader_read_finite_rt(event_reader, value);
            }
        }
        break;

        default:
            rassert(false);
    }

    if (!Streader_match_char(event_reader, ']'))
        return false;

    // Fire
    const bool is_at_global_breakpoint = true;
    const int32_t frame_offset = 0;
    const bool skip = false;
    const bool external = true;
    Player_process_event(
            player,
            ch_num,
            event_name,
            value,
            is_at_global_breakpoint,
            frame_offset,
            skip,
            external);

    // Check and perform goto if needed
    Player_check_perform_goto(player);

    // Store event parameters if processing was suspended
    if (Event_buffer_is_skipping(player->event_buffer))
    {
        player->susp_event_ch = ch_num;
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

    if (player->thread_count > 1)
    {
        // Initialised threads are waiting on vgroups_start_barrier
        player->stop_threads = true;
        Barrier_wait(&player->vgroups_start_barrier);
        for (int i = 0; i < KQT_THREADS_MAX; ++i)
        {
            if (!Thread_is_initialised(&player->threads[i]))
                continue;

            Thread_join(&player->threads[i]);
        }
    }

    Condition_deinit(&player->start_cond);

    Barrier_deinit(&player->vgroups_start_barrier);
    Barrier_deinit(&player->vgroups_finished_barrier);
    Barrier_deinit(&player->mixed_start_barrier);
    Barrier_deinit(&player->mixed_level_finished_barrier);

    del_Event_handler(player->event_handler);
    del_Mixed_signal_plan(player->mixed_signal_plan);
    del_Voice_pool(player->voices);
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        del_Channel(player->channels[i]);
    Master_params_deinit(&player->master_params);
    for (int i = 0; i < KQT_THREADS_MAX; ++i)
        Player_thread_params_deinit(&player->thread_params[i]);
    del_Event_buffer(player->event_buffer);
    del_Env_state(player->estate);
    del_Device_states(player->device_states);

    /*
    {
        const double voice_time_s = (double)voice_time / (double)CLOCKS_PER_SEC;
        const double mixed_time_s = (double)mixed_time / (double)CLOCKS_PER_SEC;
        const double total_time_s = (double)total_time / (double)CLOCKS_PER_SEC;

        fprintf(stderr, "Voice time: %.2f seconds (%.1f%% of total)\n",
                voice_time_s, 100 * voice_time_s / total_time_s);
        fprintf(stderr, "Mixed time: %.2f seconds (%.1f%% ot total)\n",
                mixed_time_s, 100 * mixed_time_s / total_time_s);
        fprintf(stderr, "Total time: %.2f seconds (%.1f%% signal processing time)\n",
                total_time_s, 100 * (voice_time_s + mixed_time_s) / total_time_s);

        fprintf(stderr, "Peak voice utilisation: %.1f%%\n",
                100.0 * (double)voice_time_max / (double)total_time_at_voice_max);
    }
    */

    memory_free(player->audio_buffer);

    memory_free(player);
    return;
}


