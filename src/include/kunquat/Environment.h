

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef KQT_ENVIRONMENT_H
#define KQT_ENVIRONMENT_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>


/**
 * \defgroup Environment Controlling interactivity environment
 *
 * \{
 *
 * \brief
 * This module describes the API for controlling the interactivity
 * environment inside a Kunquat composition.
 *
 * The environment interface of Kunquat exists to provide realtime
 * communication between composition and the environment where the composition
 * is played. The interface enables a musical score and the playback
 * environment to react to changes in one another. The environment interface
 * does not provide basic playback functionalities, but is designed to be used
 * alongside the player interface.
 *
 * Environment interface supports two types of variables: booleans and
 * integers. Both the environment and the musical score can set variables.
 * However, the environment can only read variables set by the musical score,
 * and the musical score can only read variables set by the environment. This
 * separation was added to prevent Kunquat songs from becomming Turing
 * complete. This implementation choice should also encourage design of
 * appropriate hooks for reactive musical scores rather than dumping huge
 * amounts of external state on Kunquat. The variable names should be
 * valid UTF-8 strings.
 *
 * Usage examples
 *
 * Computer games:
 *
 * - battle begins: kqt_Handle_turn_on("battle")
 * - battle ends: kqt_Handle_turn_off("battle")
 *
 * - should the groundhog put it's hands on ears?:
 *       kqt_Handle_check("violinsolo")
 *
 * - monster moves closer to player:
 *       kqt_Handle_set_int("monsterdistance", 34)
 *
 * - how fast should the soldiers be marching?:
 *       kqt_Handle_get_int("playbacktempo")
 *
 * Real life situations:
 *
 * - happy hour begins: kqt_Handle_turn_on("happyhour")
 * - happy hour ends: kqt_Handle_turn_off("happyhour")
 *
 * - should the live choir be singing?: kqt_Handle_check("choir")
 *
 * - a new customer enters the restaurant:
 *       kqt_Handle_set_int("customers", (customers / seats) * 256)
 *
 * - how fast should the lights in disco go on and off?:
 *       kqt_Handle_get_int("playbacktempo")
 */


/**
 * Sets a boolean variable to true.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param name     The name of the variable -- should not be \c NULL.
 *
 * \return   \c 1 if successful, or \c 0 if arguments were invalid.
 */
int kqt_Handle_turn_on(kqt_Handle* handle, char* name);


/**
 * Sets a boolean variable to false.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param name     The name of the variable -- should not be \c NULL.
 *
 * \return   \c 1 if successful, or \c 0 if arguments were invalid.
 */
int kqt_Handle_turn_off(kqt_Handle* handle, char* name);


/**
 * Gets the value of a boolean variable.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param name     The name of the variable -- should not be \c NULL.
 *
 * \return   The boolean value. FIXME: error situation
 */
bool kqt_Handle_check(kqt_Handle* handle, char* name);


/**
 * Sets the value of an integer variable.
 *
 * Valid range for the integer value is 0 to 256 (inclusive).
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param name     The name of the variable -- should not be \c NULL.
 * \param value    The value of the integer -- should be >= \c 0 and
 *                 <= \c 256.
 *
 * \return   \c 1 if successful, or \c 0 if arguments were invalid.
 */
int kqt_Handle_set_int(kqt_Handle* handle, char* name, int value);


/**
 * Gets the value of an integer variable.
 *
 * \param handle   The Handle -- should not be \c NULL.
 * \param name     The name of the variable -- should not be \c NULL.
 *
 * \return   The integer value.
 */
int kqt_Handle_get_int(kqt_Handle* handle, char* name);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_ENVIRONMENT_H


