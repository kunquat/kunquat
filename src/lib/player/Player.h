

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

#include <Module.h>


typedef struct Player Player;


/**
 * Creates a new Player.
 *
 * \param module   The Module -- must not be \c NULL.
 *
 * \return   The new Player if successful, or \c NULL if memory allocation
 *           failed.
 */
Player* new_Player(const Module* module);


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
 * Resets the Player state.
 *
 * \param player   The Player -- must not be \c NULL.
 */
void Player_reset(Player* player);


/**
 * Plays music.
 *
 * \param player    The Player -- must not be \c NULL.
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
void Player_skip(Player* player, int32_t nframes);


/**
 * Gets the number of frames available in the internal audio chunk.
 *
 * \param player   The Player -- must not be \c NULL.
 *
 * \return   The number of frames available.
 */
int32_t Player_get_frames_available(Player* player);


/**
 * Returns an internal audio buffer.
 *
 * \param player    The Player -- must not be \c NULL.
 * \param channel   The channel number -- must be \c 0 or \c 1.
 *
 * \return   The audio buffer.
 */
const float* Player_get_audio(Player* player, int channel);


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
bool Player_has_stopped(Player* player);


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


