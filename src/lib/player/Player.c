

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


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <Connections_search.h>
#include <Device_node.h>
#include <math_common.h>
#include <memory.h>
#include <Pat_inst_ref.h>
#include <player/Player.h>
#include <player/Player_private.h>
#include <player/Player_seq.h>
#include <player/Position.h>
#include <string_common.h>
#include <xassert.h>


static void Player_update_sliders_and_lfos_audio_rate(Player* player)
{
    assert(player != NULL);

    const int32_t rate = player->audio_rate;

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel_state* ch = player->channels[i];
        LFO_set_mix_rate(&ch->vibrato, rate);
        LFO_set_mix_rate(&ch->tremolo, rate);
        Slider_set_mix_rate(&ch->panning_slider, rate);
        LFO_set_mix_rate(&ch->autowah, rate);
    }

    Master_params* mp = &player->master_params;
    Slider_set_mix_rate(&mp->volume_slider, rate);

    return;
}


Player* new_Player(
        const Module* module,
        int32_t audio_rate,
        int32_t audio_buffer_size,
        size_t event_buffer_size,
        int voice_count)
{
    assert(module != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);
    assert(audio_buffer_size <= KQT_AUDIO_BUFFER_SIZE_MAX);
    assert(voice_count >= 0);
    assert(voice_count < KQT_VOICES_MAX);

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
    player->env = NULL;
    player->event_buffer = NULL;
    player->voices = NULL;
    Master_params_preinit(&player->master_params);
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        player->channels[i] = NULL;
    player->event_handler = NULL;

    player->frame_remainder = 0.0;

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        Cgiter_init(&player->cgiters[i], player->module, i);

    player->audio_frames_processed = 0;
    player->nanoseconds_history = 0;

    player->events_returned = false;

    // Init fields
    player->device_states = new_Device_states();
    player->env = player->module->env; //new_Environment(); // TODO
    player->event_buffer = new_Event_buffer(event_buffer_size);
    player->voices = new_Voice_pool(voice_count);
    if (player->device_states == NULL ||
            player->env == NULL ||
            player->event_buffer == NULL ||
            player->voices == NULL ||
            !Voice_pool_reserve_state_space(
                player->voices,
                sizeof(Voice_state)))
    {
        del_Player(player);
        return NULL;
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        player->channels[i] = new_Channel_state(
                i,
                Module_get_insts(player->module),
                player->env,
                player->voices,
                &player->master_params.tempo,
                &player->audio_rate);
        if (player->channels[i] == NULL)
        {
            del_Player(player);
            return NULL;
        }
    }

    if (Master_params_init(&player->master_params, player->module) == NULL)
    {
        del_Player(player);
        return NULL;
    }

    Player_update_sliders_and_lfos_audio_rate(player);

    player->event_handler = new_Event_handler(
            &player->master_params,
            player->channels,
            Module_get_insts(player->module),
            Module_get_effects(player->module));
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
    assert(player != NULL);
    return player->event_handler;
}


Device_states* Player_get_device_states(const Player* player)
{
    assert(player != NULL);
    return player->device_states;
}


bool Player_reserve_voice_state_space(Player* player, size_t size)
{
    assert(player != NULL);
    return Voice_pool_reserve_state_space(player->voices, size);
}


bool Player_add_channel_gen_state_key(Player* player, const char* key)
{
    assert(player != NULL);
    assert(key != NULL);

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        if (!Channel_gen_state_set_key(player->channels[i]->cgstate, key))
            return false;
    }

    return true;
}


void Player_reset(Player* player)
{
    assert(player != NULL);

    // TODO: playback mode and start pos as arguments

    Master_params_reset(&player->master_params);

    Player_update_sliders_and_lfos_audio_rate(player);
    Player_update_sliders_and_lfos_tempo(player);

    player->frame_remainder = 0.0;

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);
    }

    Event_buffer_clear(player->event_buffer);

    player->audio_frames_processed = 0;
    player->nanoseconds_history = 0;

    player->events_returned = false;

    return;
}


bool Player_set_audio_rate(Player* player, int32_t rate)
{
    assert(player != NULL);
    assert(rate > 0);

    if (player->audio_rate == rate)
        return true;

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
    assert(player != NULL);
    return player->audio_rate;
}


bool Player_set_audio_buffer_size(Player* player, int32_t size)
{
    assert(player != NULL);
    assert(size >= 0);

    if (player->audio_buffer_size == size)
        return true;

    // Reduce supported size (in case we fail memory allocation)
    player->audio_buffer_size = MIN(player->audio_buffer_size, size);

    // Handle empty buffers
    if (player->audio_buffer_size == 0)
    {
        for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        {
            memory_free(player->audio_buffers[i]);
            player->audio_buffers[i] = NULL;
        }
        return true;
    }

    // Reallocate buffers
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        float* new_buffer = memory_realloc_items(
                float,
                size,
                player->audio_buffers[i]);
        if (new_buffer == NULL)
            return false;

        player->audio_buffers[i] = new_buffer;
    }

    // Set final supported buffer size
    player->audio_buffer_size = size;

    return true;
}


int32_t Player_get_audio_buffer_size(const Player* player)
{
    assert(player != NULL);
    return player->audio_buffer_size;
}


int64_t Player_get_nanoseconds(const Player* player)
{
    assert(player != NULL);

    const int32_t ns_this_audio_rate =
        player->audio_frames_processed * 1000000000LL / player->audio_rate;
    return player->nanoseconds_history + ns_this_audio_rate;
}


static void Player_process_voices(
        Player* player,
        int32_t render_start,
        int32_t nframes)
{
    assert(player != NULL);
    assert(render_start >= 0);
    assert(nframes >= 0);

    if (nframes == 0)
        return;

    const int32_t render_stop = render_start + nframes;

    // Foreground voices
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Channel_state* ch = player->channels[i];
        for (int k = 0; k < KQT_GENERATORS_MAX; ++k)
        {
            if (ch->fg[k] != NULL)
            {
                // Verify voice ownership
                ch->fg[k] = Voice_pool_get_voice(
                        player->voices,
                        ch->fg[k],
                        ch->fg_id[k]);

                if (ch->fg[k] != NULL)
                {
                    // Render
                    assert(ch->fg[k]->prio > VOICE_PRIO_INACTIVE);
                    Voice_mix(
                            ch->fg[k],
                            render_stop,
                            render_start,
                            player->audio_rate,
                            player->master_params.tempo);
                }
            }
        }
    }

    // Background voices
    int16_t active_voices = Voice_pool_mix_bg(
            player->voices,
            render_stop,
            render_start,
            player->audio_rate,
            player->master_params.tempo);

    player->master_params.active_voices =
        MAX(player->master_params.active_voices, active_voices);
}


void Player_play(Player* player, int32_t nframes)
{
    assert(player != NULL);
    assert(player->audio_buffer_size > 0);
    assert(nframes >= 0);

    Event_buffer_clear(player->event_buffer);

    nframes = MIN(nframes, player->audio_buffer_size);

    // TODO: separate data and playback state in connections
    Connections* connections = player->module->connections;

    Device_states_clear_audio_buffers(player->device_states, 0, nframes);
    if (connections != NULL)
    {
        Connections_clear_buffers(connections, 0, nframes);
    }

    // TODO: check if song or pattern instance location has changed

    // Composition-level progress
    const bool was_playing = !Player_has_stopped(player);
    int32_t rendered = 0;
    while (rendered < nframes)
    {
        // Move forwards in composition
        int32_t to_be_rendered = nframes - rendered;
        if (!player->master_params.parent.pause && !Player_has_stopped(player))
        {
            to_be_rendered = Player_move_forwards(player, to_be_rendered, false);
        }

        // Don't add padding audio if stopped during this call
        if (was_playing && Player_has_stopped(player))
        {
            assert(to_be_rendered == 0);
            break;
        }

        // Process voices
        Player_process_voices(player, rendered, to_be_rendered);

        // Update panning slides, TODO: revisit
        for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        {
            Channel_state* ch = player->channels[i];
            if (Slider_in_progress(&ch->panning_slider))
                Slider_skip(&ch->panning_slider, to_be_rendered);
        }

        // Process connection graph
        if (connections != NULL)
        {
            Connections_mix(
                    connections,
                    player->device_states,
                    rendered,
                    rendered + to_be_rendered,
                    player->audio_rate,
                    player->master_params.tempo);

            // Get access to mixed output
            Device* master = Device_node_get_device(
                    Connections_get_master(connections));
            Audio_buffer* buffer = Device_get_buffer(
                    master,
                    DEVICE_PORT_TYPE_RECEIVE,
                    0);
            assert(buffer != NULL);

            kqt_frame* bufs[] =
            {
                Audio_buffer_get_buffer(buffer, 0),
                Audio_buffer_get_buffer(buffer, 1),
            };
            assert(bufs[0] != NULL);
            assert(bufs[1] != NULL);

            // Apply master volume
            for (int32_t i = rendered; i < rendered + to_be_rendered; ++i)
            {
                if (Slider_in_progress(&player->master_params.volume_slider))
                    player->master_params.volume = Slider_step(
                            &player->master_params.volume_slider);

                for (int ch = 0; ch < 2; ++ch)
                    bufs[ch][i] *= player->master_params.volume;
            }
        }

        rendered += to_be_rendered;
    }

    if (connections != NULL)
    {
        Device* master = Device_node_get_device(
                Connections_get_master(connections));
        Audio_buffer* buffer = Device_get_buffer(
                master,
                DEVICE_PORT_TYPE_RECEIVE,
                0);
        if (buffer != NULL)
        {
            // Apply render volume
            kqt_frame* bufs[] =
            {
                Audio_buffer_get_buffer(buffer, 0),
                Audio_buffer_get_buffer(buffer, 1),
            };
            assert(bufs[0] != NULL);
            assert(bufs[1] != NULL);
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int32_t i = 0; i < rendered; ++i)
                {
                    player->audio_buffers[ch][i] =
                        bufs[ch][i] * player->module->mix_vol;
                }
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
    assert(player != NULL);
    assert(nframes >= 0);

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
        // Move forwards in composition
        int32_t to_be_skipped = MIN(nframes - skipped, INT32_MAX);
        to_be_skipped = Player_move_forwards(player, to_be_skipped, true);

        if (Player_has_stopped(player))
        {
            assert(to_be_skipped == 0);
            break;
        }

        // Update master volume slider
        Slider_skip(&player->master_params.volume_slider, to_be_skipped);

        skipped += to_be_skipped;
    }

    player->audio_frames_processed += skipped;

    player->events_returned = false;

    return;
}


int32_t Player_get_frames_available(const Player* player)
{
    assert(player != NULL);
    return player->audio_frames_available;
}


const float* Player_get_audio(const Player* player, int channel)
{
    assert(player != NULL);
    assert(channel == 0 || channel == 1);
    return player->audio_buffers[channel];
}


const char* Player_get_events(Player* player)
{
    assert(player != NULL);

    if (player->events_returned)
    {
        // Event buffer contains old data, clear
        Event_buffer_clear(player->event_buffer);

        // TODO: get more events if needed
    }

    player->events_returned = true;

    return Event_buffer_get_events(player->event_buffer);
}


bool Player_has_stopped(const Player* player)
{
    assert(player != NULL);
    return (player->master_params.playback_state == PLAYBACK_STOPPED);
}


bool Player_fire(Player* player, int ch, char* event_desc, Read_state* rs)
{
    assert(player != NULL);
    assert(ch >= 0);
    assert(ch < KQT_CHANNELS_MAX);
    assert(event_desc != NULL);
    assert(rs != NULL);

    if (rs->error)
        return false;

    Event_buffer_clear(player->event_buffer);

    const Event_names* event_names = Event_handler_get_names(player->event_handler);

    char event_name[EVENT_NAME_MAX + 1] = "";
    Event_type type = Event_NONE;

    // Get event name
    event_desc = get_event_type_info(
            event_desc,
            event_names,
            rs,
            event_name,
            &type);
    if (rs->error)
        return false;

    // Get event argument
    Value* value = VALUE_AUTO;
    value->type = Event_names_get_param_type(event_names, event_name);

    switch (value->type)
    {
        case VALUE_TYPE_NONE:
            event_desc = read_null(event_desc, rs);
            break;

        case VALUE_TYPE_BOOL:
            event_desc = read_bool(event_desc, &value->value.bool_type, rs);
            break;

        case VALUE_TYPE_INT:
            event_desc = read_int(event_desc, &value->value.int_type, rs);
            break;

        case VALUE_TYPE_FLOAT:
            event_desc = read_double(event_desc, &value->value.float_type, rs);
            break;

        case VALUE_TYPE_TSTAMP:
            event_desc = read_tstamp(event_desc, &value->value.Tstamp_type, rs);
            break;

        case VALUE_TYPE_STRING:
        {
            event_desc = read_string(
                    event_desc,
                    value->value.string_type,
                    ENV_VAR_NAME_MAX,
                    rs);
        }
        break;

        case VALUE_TYPE_PAT_INST_REF:
        {
            event_desc = read_pat_inst_ref(
                    event_desc,
                    &value->value.Pat_inst_ref_type,
                    rs);
        }
        break;

        default:
            assert(false);
    }

    event_desc = read_const_char(event_desc, ']', rs);
    if (rs->error)
        return false;

    // Fire
    if (!Event_handler_trigger(
            player->event_handler,
            ch,
            event_name,
            value))
    {
        return false;
    }

    // Add event to buffer
    Event_buffer_add(player->event_buffer, ch, event_name, value);
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
        del_Channel_state(player->channels[i]);
    Master_params_deinit(&player->master_params);
    del_Event_buffer(player->event_buffer);
    //del_Environment(player->env); // TODO
    del_Device_states(player->device_states);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        memory_free(player->audio_buffers[i]);

    memory_free(player);
    return;
}


