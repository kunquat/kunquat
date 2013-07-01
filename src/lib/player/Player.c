

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

#include <Channel_state.h>
#include <Connections_search.h>
#include <Device_node.h>
#include <Environment.h>
#include <Event_handler.h>
#include <math_common.h>
#include <memory.h>
#include <Pat_inst_ref.h>
#include <player/Cgiter.h>
#include <player/Master_params.h>
#include <player/Player.h>
#include <player/Position.h>
#include <Tstamp.h>
#include <Voice_pool.h>
#include <xassert.h>


struct Player
{
    const Module* module;

    int32_t audio_rate;
    int32_t audio_chunk_size;
    float*  audio_buffers[2];
    int32_t audio_frames_available;

    Environment*   env;
    Voice_pool*    voices;
    Master_params  master_params;
    Channel_state* channels[KQT_CHANNELS_MAX];
    Event_handler* event_handler;

    double frame_remainder; // used for sub-frame time tracking

    Cgiter cgiters[KQT_CHANNELS_MAX];
};


Player* new_Player(const Module* module)
{
    assert(module != NULL);

    Player* player = memory_alloc_item(Player);
    if (player == NULL)
        return NULL;

    // Sanitise fields
    player->module = module;

    player->audio_rate = 48000;
    player->audio_chunk_size = 2048;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        player->audio_buffers[i] = NULL;
    player->audio_frames_available = 0;

    player->env = NULL;
    player->voices = NULL;
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        player->channels[i] = NULL;
    player->event_handler = NULL;

    player->frame_remainder = 0.0;

    memset(player->cgiters, 0, sizeof(player->cgiters));

    // Init fields
    player->env = new_Environment();
    player->voices = new_Voice_pool(256);
    if (player->env == NULL ||
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

    if (Master_params_init(&player->master_params, player->env) == NULL)
    {
        del_Player(player);
        return NULL;
    }

    player->event_handler = new_Event_handler(
            &player->master_params,
            NULL,
            player->channels,
            Module_get_insts(player->module),
            Module_get_effects(player->module));
    if (player->event_handler == NULL)
    {
        del_Player(player);
        return NULL;
    }

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        player->audio_buffers[i] = memory_alloc_items(
                float, player->audio_chunk_size);
        if (player->audio_buffers[i] == NULL)
        {
            del_Player(player);
            return NULL;
        }
    }

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter_init(&player->cgiters[i], player->module, i);
    }

    return player;
}


void Player_reset(Player* player)
{
    assert(player != NULL);

    // TODO: playback mode and start pos as arguments

    Master_params_reset(&player->master_params, player->module);

    player->frame_remainder = 0.0;

    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter_reset(&player->cgiters[i], &player->master_params.cur_pos);
    }

    return;
}


bool Player_set_audio_rate(Player* player, int32_t rate)
{
    assert(player != NULL);
    assert(rate > 0);

    player->audio_rate = rate;

    return true;
}


static int32_t Player_process_cgiters(Player* player, int32_t nframes)
{
    assert(player != NULL);
    assert(!Player_has_stopped(player));
    assert(nframes >= 0);

    int32_t to_be_rendered = nframes;

    // Get maximum duration to move forwards
    Tstamp* limit = Tstamp_fromframes(
            TSTAMP_AUTO,
            nframes,
            player->master_params.tempo,
            player->audio_rate);

    // Process cgiters at current position
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
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
            while (el->event != NULL)
            {
                const bool success = Event_handler_trigger(
                        player->event_handler,
                        i,
                        Event_get_desc(el->event),
                        false, // not silent
                        NULL);

                (void)success;

                el = el->next;
            }
        }

        // See how much we can move forwards
        Tstamp* dist = Tstamp_copy(TSTAMP_AUTO, limit);
        if (Cgiter_peek(cgiter, dist) && Tstamp_cmp(dist, limit) < 0)
            Tstamp_copy(limit, dist);
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
        player->master_params.playback_state = PLAYBACK_STOPPED;
        return 0;
    }

    // Get actual number of frames to be rendered
    double dframes = Tstamp_toframes(
            limit,
            player->master_params.tempo,
            player->audio_rate);
    assert(dframes >= 0.0);

    to_be_rendered = (int32_t)dframes;
    player->frame_remainder += dframes - to_be_rendered;
    if (player->frame_remainder > 0.5)
    {
        ++to_be_rendered;
        player->frame_remainder -= 1.0;
    }

    assert(to_be_rendered <= nframes);

    return to_be_rendered;
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

    // TODO: Update tempo & freq on sliders

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
    assert(nframes >= 0);

    nframes = MIN(nframes, player->audio_chunk_size);

    // TODO: separate data and playback state in connections
    Connections* connections = player->module->connections;

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
        // Process cgiters
        int32_t to_be_rendered = nframes - rendered;
        if (!player->master_params.parent.pause && !Player_has_stopped(player))
        {
            to_be_rendered = Player_process_cgiters(player, to_be_rendered);
        }

        // Don't add padding audio if stopped during this call
        if (was_playing && Player_has_stopped(player))
            break;

        // Process voices
        Player_process_voices(player, rendered, to_be_rendered);

        // Process connection graph
        if (connections != NULL)
        {
            Connections_mix(
                    connections,
                    rendered,
                    to_be_rendered,
                    player->audio_rate,
                    player->master_params.tempo);
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
            for (int i = 0; i < 2; ++i)
            {
                for (int32_t k = 0; k < rendered; ++k)
                {
                    player->audio_buffers[i][k] =
                        bufs[i][k] * player->module->mix_vol;
                }
            }
        }
    }

    player->audio_frames_available = rendered;

    return;
}


int32_t Player_get_frames_available(Player* player)
{
    assert(player != NULL);
    return player->audio_frames_available;
}


float* Player_get_audio(Player* player, int channel)
{
    assert(player != NULL);
    assert(channel == 0 || channel == 1);
    return player->audio_buffers[channel];
}


bool Player_has_stopped(Player* player)
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

    return Event_handler_trigger_const(
            player->event_handler,
            ch,
            event_desc,
            false,
            rs);
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
    del_Environment(player->env);

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        memory_free(player->audio_buffers[i]);

    memory_free(player);
    return;
}


