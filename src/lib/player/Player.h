

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


#ifndef K_PLAYER_H
#define K_PLAYER_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <File_base.h>
#include <kunquat/limits.h>
#include <Module.h>
#include <player/Event_handler.h>


#define DEFAULT_AUDIO_RATE 48000
#define DEFAULT_TEMPO 120.0


typedef struct Player Player;


/**
 * Creates a new Player.
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
 * Returns the event handler of the Player. TODO: this is a hack
 */
const Event_handler* Player_get_event_handler(const Player* player);


/**
 * Returns the Device state collection of the Player.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The Device states.
 */
Device_states* Player_get_device_states(const Player* player);


/**
 * Reserves state space for internal voice pool.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param size     The new size.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_reserve_voice_state_space(Player* player, size_t size);


/**
 * Allocates memory for a list of Channel-specific generator variables.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param str      The list of keys as a JSON string, or \c NULL. TODO: make const
 * \param rs       The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a rs will _not_ be
 *           modified if memory allocation failed.
 */
bool Player_alloc_channel_gen_state_keys(
        Player* player,
        char* str,
        Read_state* rs);


/**
 * Refreshes environment state.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_refresh_env_state(Player* player);


/**
 * Refreshes bind state.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_refresh_bind_state(Player* player);


/**
 * Sets audio rate.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param rate     The audio rate in frames per second -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_set_audio_rate(Player* player, int32_t rate);


/**
 * Gets audio rate.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The audio rate.
 */
int32_t Player_get_audio_rate(const Player* player);


/**
 * Sets audio buffer size.
 *
 * \param player   The Player -- must not be \c NULL.
 * \param size     The new audio buffer size -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Player_set_audio_buffer_size(Player* player, int32_t size);


/**
 * Gets audio buffer size.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The audio buffer size.
 */
int32_t Player_get_audio_buffer_size(const Player* player);


/**
 * Returns the length of music rendered after the last reset.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The length in nanoseconds.
 */
int64_t Player_get_nanoseconds(const Player* player);


/**
 * Resets the Player state.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void Player_reset(Player* player);


/**
 * Plays music.
 *
 * \param player    The Player -- must not be \c NULL and must have audio
 *                  buffers of positive size.
 * \param nframes   The number of frames to be rendered -- must be >= \c 0.
 *                  The actual number of frames rendered may be anything
 *                  between \c 0 and \a nframes.
 */
void Player_play(Player* player, int32_t nframes);


/**
 * Skips music.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param nframes   The number of frames to be skipped -- must be >= \c 0.
 */
void Player_skip(Player* player, int64_t nframes);


/**
 * Gets the number of frames available in the internal audio chunk.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The number of frames available.
 */
int32_t Player_get_frames_available(const Player* player);


/**
 * Returns an internal audio buffer.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param channel   The channel number -- must be \c 0 or \c 1.
 *
 * \return   The audio buffer.
 */
const float* Player_get_audio(const Player* player, int channel);


/**
 * Returns an internal event buffer.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The event buffer.
 */
const char* Player_get_events(Player* player);


/**
 * Tells whether the Player has reached the end of playback.
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
 * Fires an event.
 *
 * \param player       The Player -- must not be \c NULL.
 * \param ch           The channel number -- must be >= \c 0 and
 *                     < \c KQT_CHANNELS_MAX.
 * \param event_desc   The event description -- must not be \c NULL.
 * \param rs           The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if \a event_desc was invalid.
 */
bool Player_fire(Player* player, int ch, char* event_desc, Read_state* rs);


/**
 * Destroys the Player.
 *
 * \param player   The Player, or \c NULL.
 */
void del_Player(Player* player);


#endif // K_PLAYER_H


