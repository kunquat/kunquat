

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
#include <transient/Position.h>
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
    const Module* module;

    int32_t audio_rate;
    int32_t audio_chunk_size;
    float*  audio_buffers[2];
    int32_t audio_frames_available;

    Playback_state state;
    bool is_paused;

    Position cur_pos;
    double frame_remainder; // used for sub-frame time tracking

    Cgiter cgiters[KQT_CHANNELS_MAX];

    float tempo;
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

    player->state = PLAYBACK_SONG;
    player->is_paused = false;

    Position_init(&player->cur_pos);
    player->frame_remainder = 0.0;

    memset(player->cgiters, 0, sizeof(player->cgiters));

    player->tempo = 120;

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
        Cgiter_init(&player->cgiters[i], player->module);
    }

    return player;
}


void Player_reset(Player* player)
{
    assert(player != NULL);

    // TODO: playback mode and start pos as arguments

    player->frame_remainder = 0.0;

    Position start_pos;
    Position_init(&start_pos);
    start_pos.track = 0;
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
    {
        Cgiter_reset(&player->cgiters[i], &start_pos);
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
            player->tempo,
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
            // TODO: process trigger row
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
        player->state = PLAYBACK_STOPPED;
        return 0;
    }

    // Get actual number of frames to be rendered
    double dframes = Tstamp_toframes(
            Tstamp_sub(TSTAMP_AUTO, limit, &player->cur_pos.pat_pos),
            player->tempo,
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
        // Process cgiters
        int32_t to_be_rendered = nframes - rendered;
        if (!player->is_paused && !Player_has_stopped(player))
        {
            to_be_rendered = Player_process_cgiters(player, to_be_rendered);
        }

        // Don't add padding audio if stopped during this call
        if (was_playing && Player_has_stopped(player))
            break;

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


