

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

#include <math_common.h>
#include <memory.h>
#include <Pat_inst_ref.h>
#include <transient/Cgiter.h>
#include <transient/Player.h>
#include <Tstamp.h>
#include <xassert.h>


typedef enum
{
    PLAYBACK_STOPPED = 0,
    PLAYBACK_PATTERN,
    PLAYBACK_SONG,
    PLAYBACK_MODULE,
    PLAYBACK_COUNT
} Playback_state;


struct Player
{
    int32_t audio_rate;
    int32_t audio_chunk_size;
    float*  audio_buffers[2];
    int32_t audio_frames_available;

    Playback_state state;
    bool is_paused;

    int16_t      cur_track;
    int16_t      cur_system;
    Tstamp       cur_pat_pos;
    Pat_inst_ref cur_piref;

    Cgiter cgiters[KQT_CHANNELS_MAX];

    float tempo;

    const Module* module;
};


Player* new_Player(const Module* module)
{
    assert(module != NULL);

    Player* player = memory_alloc_item(Player);
    if (player == NULL)
        return NULL;

    // Sanitise fields
    player->audio_rate = 48000;
    player->audio_chunk_size = 2048;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        player->audio_buffers[i] = NULL;
    player->audio_frames_available = 0;

    player->state = PLAYBACK_SONG;
    player->is_paused = false;

    player->cur_track = 0;
    player->cur_system = 0;
    Tstamp_set(&player->cur_pat_pos, 0, 0);
    player->cur_piref.pat = -1;
    player->cur_piref.inst = 0;

    memset(player->cgiters, 0, sizeof(player->cgiters));

    player->tempo = 120;

    player->module = NULL;

    // Init fields
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
        Cgiter_init(&player->cgiters[i]);
    }

    player->module = module;

    return player;
}


bool Player_set_audio_rate(Player* player, int32_t rate)
{
    assert(player != NULL);
    assert(rate > 0);

    player->audio_rate = rate;

    return true;
}


static const Pat_inst_ref* find_pat_inst_ref(
        const Module* module,
        int16_t track,
        int16_t system)
{
    const Track_list* tl = Module_get_track_list(module);
    if (tl != NULL && track < (int16_t)Track_list_get_len(tl))
    {
        const int16_t cur_song = Track_list_get_song_index(tl, track);
        const Order_list* ol = Module_get_order_list(module, cur_song);
        if (ol != NULL && system < (int16_t)Order_list_get_len(ol))
        {
            Pat_inst_ref* piref = Order_list_get_pat_inst_ref(ol, system);
            assert(piref != NULL);
            return piref;
        }
    }
    return NULL;
}


static void Player_go_to_next_system(Player* player)
{
    assert(player != NULL);

    Tstamp_set(&player->cur_pat_pos, 0, 0);

    if (player->state == PLAYBACK_PATTERN)
        return;

    ++player->cur_system;
    player->cur_piref.pat = -1;
    const Pat_inst_ref* piref = find_pat_inst_ref(
            player->module,
            player->cur_track,
            player->cur_system);
    if (piref != NULL)
        player->cur_piref = *piref;

    return;
}


static int32_t Player_process_pattern(
        Player* player,
        const Pattern* pattern,
        int32_t nframes)
{
    assert(player != NULL);
    assert(!Player_has_stopped(player));
    assert(pattern != NULL);

    const Tstamp* zero_time = TSTAMP_AUTO;

    const Tstamp* pat_length = Pattern_get_length(pattern);

    // Check repeated playback of zero-length pattern
    if (player->state == PLAYBACK_PATTERN &&
            Tstamp_cmp(zero_time, pat_length) == 0)
    {
        player->state = PLAYBACK_STOPPED;
        return 0;
    }

    // Move forwards if the cursor has reached beyond pattern end
    // (implies edit between Player_play calls)
    if (Tstamp_cmp(&player->cur_pat_pos, pat_length) > 0)
    {
        Player_go_to_next_system(player);
        return 0;
    }

    // TODO: Process events at current position

    // Move forwards if we have reached the end of pattern
    if (Tstamp_cmp(&player->cur_pat_pos, pat_length) >= 0)
    {
        Player_go_to_next_system(player);
        return 0;
    }

    int32_t to_be_rendered = nframes;

    // Get maximum duration to move forwards
    Tstamp* limit = Tstamp_fromframes(
            TSTAMP_AUTO,
            nframes,
            player->tempo,
            player->audio_rate);
    Tstamp_add(limit, limit, &player->cur_pat_pos);

    if (Tstamp_cmp(limit, pat_length) > 0)
    {
        Tstamp_copy(limit, pat_length);
        int32_t max_to_be_rendered = Tstamp_toframes(
                Tstamp_sub(TSTAMP_AUTO, limit, &player->cur_pat_pos),
                player->tempo,
                player->audio_rate);
        to_be_rendered = MIN(to_be_rendered, max_to_be_rendered);
    }

    assert(Tstamp_cmp(&player->cur_pat_pos, limit) <= 0);

    // Increment playback position
    Tstamp_copy(&player->cur_pat_pos, limit);

    return to_be_rendered;
}


void Player_play(Player* player, int32_t nframes)
{
    assert(player != NULL);
    assert(nframes >= 0);

    nframes = MIN(nframes, player->audio_chunk_size);

    // TODO: check if song or pattern instance location has changed

    // Composition-level progress
    const bool was_playing = !Player_has_stopped(player);
    int32_t rendered = 0;
    while (rendered < nframes)
    {
        // Find pattern to process
        const Pattern* pat = NULL;
        if (!player->is_paused)
        {
            switch (player->state)
            {
                case PLAYBACK_STOPPED:
                {
                }
                break;

                case PLAYBACK_SONG:
                {
                    // Find pattern
                    const Pat_inst_ref* piref = find_pat_inst_ref(
                            player->module,
                            player->cur_track,
                            player->cur_system);
                    if (piref != NULL)
                    {
                        player->cur_piref = *piref;
                        pat = Module_get_pattern(
                                player->module,
                                &player->cur_piref);
                    }

                    // Handle end of playback range
                    if (pat == NULL && !player->is_paused)
                    {
                        // TODO: Go back to start if in infinite mode
                        player->state = PLAYBACK_STOPPED;
                    }
                }
                break;

                default:
                    assert(false);
            }
        }

        // Don't add padding audio if stopped during this call
        if (was_playing && Player_has_stopped(player))
            break;

        // Process pattern contents
        int32_t to_be_rendered = nframes - rendered;
        if (pat != NULL && !player->is_paused && !Player_has_stopped(player))
        {
            to_be_rendered = Player_process_pattern(
                    player,
                    pat,
                    to_be_rendered);
        }

        // TODO: Render audio (not just silence)
        for (int32_t i = rendered; i < to_be_rendered; ++i)
        {
            player->audio_buffers[0][i] = 0.0f;
            player->audio_buffers[1][i] = 0.0f;
        }

        rendered += to_be_rendered;
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
    return (player->state == PLAYBACK_STOPPED);
}


void del_Player(Player* player)
{
    if (player == NULL)
        return;

    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
        memory_free(player->audio_buffers[i]);

    memory_free(player);
    return;
}


