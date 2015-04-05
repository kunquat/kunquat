

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PLAYER_H
#define K_PLAYER_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <kunquat/limits.h>
#include <module/Module.h>
#include <player/Event_handler.h>
#include <string/Streader.h>


#define DEFAULT_AUDIO_RATE 48000
#define DEFAULT_TEMPO 120.0


typedef struct Player Player;


/**
 * Create a new Player.
 *
 * \param module              The Module -- must not be \c NULL.
 * \param audio_rate          The audio rate -- must be > \c 0.
 * \param audio_buffer_size   The audio buffer size -- must be >= \c 0 and
 *                            <= \c KQT_AUDIO_BUFFER_SIZE_MAX.
 * \param event_buffer_size   The event buffer size.
 * \param voice_count         The number of voices allocated
 *                            -- must be >= \c 0 and <= \c KQT_VOICES_MAX.
 *
 * \return   The new Player if successful, or \c NULL if memory allocation
 *           failed.
 */
Player* new_Player(
        const Module* module,
        int32_t audio_rate,
        int32_t audio_buffer_size,
        size_t event_buffer_size,
        int voice_count);


/**
 * Return the event handler of the Player. TODO: this is a hack
 */
const Event_handler* Player_get_event_handler(const Player* player);


/**
 * Return the Device state collection of the Player.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The Device states.
 */
Device_states* Player_get_device_states(const Player* player);


/**
 * Reserve state space for internal voice pool.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param size     The new size.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_reserve_voice_state_space(Player* player, size_t size);


/**
 * Allocate memory for a list of Channel-specific processor variables.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param sr       The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Player_alloc_channel_proc_state_keys(Player* player, Streader* sr);


/**
 * Refresh environment state.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_refresh_env_state(Player* player);


/**
 * Refresh bind state.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_refresh_bind_state(Player* player);


/**
 * Set audio rate.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param rate     The audio rate in frames per second -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_set_audio_rate(Player* player, int32_t rate);


/**
 * Get audio rate.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The audio rate.
 */
int32_t Player_get_audio_rate(const Player* player);


/**
 * Set audio buffer size.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param size     The new audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_set_audio_buffer_size(Player* player, int32_t size);


/**
 * Get audio buffer size.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The audio buffer size.
 */
int32_t Player_get_audio_buffer_size(const Player* player);


/**
 * Return the length of music rendered or skipped after the last reset.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
int64_t Player_get_nanoseconds(const Player* player);


/**
 * Reset the Player state.
 *
 * \param player      The Player -- must not be \c NULL.
 * \param track_num   The track number, or \c -1 to indicate all tracks.
 */
void Player_reset(Player* player, int16_t track_num);


/**
 * Play music.
 *
 * \param player    The Player -- must not be \c NULL and must have audio
 *                  buffers of positive size.
 * \param nframes   The number of frames to be rendered -- must be >= \c 0.
 *                  The actual number of frames rendered may be anything
 *                  between \c 0 and \a nframes.
 */
void Player_play(Player* player, int32_t nframes);


/**
 * Skip music.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param nframes   The number of frames to be skipped -- must be >= \c 0.
 */
void Player_skip(Player* player, int64_t nframes);


/**
 * Get the number of frames available in the internal audio chunk.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The number of frames available.
 */
int32_t Player_get_frames_available(const Player* player);


/**
 * Return an internal audio buffer.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param channel   The channel number -- must be \c 0 or \c 1.
 *
 * \return   The audio buffer.
 */
const float* Player_get_audio(const Player* player, int channel);


/**
 * Return an internal event buffer.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The event buffer.
 */
const char* Player_get_events(Player* player);


/**
 * Tell whether the Player has reached the end of playback.
 *
 * It is still possible to render more audio, in which case the Player
 * will act as if paused at the end of composition.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   \c true if end has been reached, otherwise \c false.
 */
bool Player_has_stopped(const Player* player);


/**
 * Fire an event.
 *
 * \param player         The Player -- must not be \c NULL.
 * \param ch             The channel number -- must be >= \c 0 and
 *                       < \c KQT_CHANNELS_MAX.
 * \param event_reader   The event reader -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if the event was invalid.
 */
bool Player_fire(Player* player, int ch, Streader* event_reader);


/**
 * Destroy the Player.
 *
 * \param player   The Player, or \c NULL.
 */
void del_Player(Player* player);


#endif // K_PLAYER_H


