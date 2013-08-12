

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


#ifndef K_DEVICE_IMPL_H
#define K_DEVICE_IMPL_H


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <AAtree.h>
#include <Decl.h>
#include <player/Device_state.h>
#include <Tstamp.h>


#define DEVICE_KEY_INDICES_MAX 8


/**
 * The base class of Generator and DSP implementations.
 */
struct Device_impl
{
    Device* device;
    AAtree* update_cbs;

    void (*destroy)(Device_impl* dimpl);
};


/**
 * Initialises the Device implementation.
 *
 * \param dimpl     The Device implementation -- must not be \c NULL.
 * \param destroy   The destructor -- must not be \c NULL.
 * \param audio_rate          The audio rate. TODO: remove
 * \param audio_buffer_size   The audio buffer size. TODO: remove
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_init(
        Device_impl* dimpl,
        void (*destroy)(Device_impl* dimpl),
        int32_t audio_rate, int32_t audio_buffer_size);


/**
 * Registers boolean value update functions.
 *
 * The key pattern may contain \c 0 to \c DEVICE_KEY_INDICES_MAX sequences of
 * XX* which are matched against hexadecimal numbers of actual keys. Example:
 *
 *    voice_XX/param_XXX/p_volume.jsonf
 *
 * Any indices with the correct amount of hexadecimal digits (here, 2 and 3)
 * are extracted from the actual updated key and passed as an array of
 * integers to the callback functions.
 *
 * An XX* sequence must be followed by a forward slash '/'.
 *
 * WARNING: A literal part of your key must not contain a valid hexadecimal
 * number followed by a forward slash '/', Example:
 *
 *    echo_eff/tap_XX/p_volume.jsonf <- Invalid: eff is interpreted as hex
 *    echo_XXX/tap_XX/p_volume.jsonf <- The above would get confused with this
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param default_val    The default value passed to callbacks.
 * \param update_dev     The Device implementation update callback function
 *                       -- must not be \c NULL.
 * \param update_state   The Device state update callback function, or
 *                       \c NULL if not required for the key pattern.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_bool(
        Device_impl* dimpl,
        const char* keyp,
        bool default_val,
        bool (*update)(Device_impl*, int32_t[DEVICE_KEY_INDICES_MAX], bool),
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            bool));


/**
 * Registers float value update functions.
 *
 * See \a Device_impl_register_update_bool for detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param default_val    The default value passed to callbacks.
 * \param update_dev     The Device implementation update callback function
 *                       -- must not be \c NULL.
 * \param update_state   The Device state update callback function, or
 *                       \c NULL if not required for the key pattern.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_float(
        Device_impl* dimpl,
        const char* keyp,
        double default_val,
        bool (*update)(Device_impl*, int32_t[DEVICE_KEY_INDICES_MAX], double),
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            double));


/**
 * Registers integer value update functions.
 *
 * See \a Device_impl_register_update_bool for detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param default_val    The default value passed to callbacks.
 * \param update_dev     The Device implementation update callback function
 *                       -- must not be \c NULL.
 * \param update_state   The Device state update callback function, or
 *                       \c NULL if not required for the key pattern.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_int(
        Device_impl* dimpl,
        const char* keyp,
        int64_t default_val,
        bool (*update)(Device_impl*, int32_t[DEVICE_KEY_INDICES_MAX], int64_t),
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            int64_t));


/**
 * Registers timestamp value update functions.
 *
 * See \a Device_impl_register_update_bool for detailed description of the
 * \a keyp argument.
 *
 * \param dimpl          The Device implementation -- must not be \c NULL.
 * \param keyp           The key pattern -- must not be \c NULL.
 * \param default_val    The default value passed to callbacks
 *                       -- must not be \c NULL
 * \param update_dev     The Device implementation update callback function
 *                       -- must not be \c NULL.
 * \param update_state   The Device state update callback function, or
 *                       \c NULL if not required for the key pattern.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_impl_register_update_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        const Tstamp* default_val,
        bool (*update)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*),
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*));


/**
 * Updates a key in the Device implementation.
 *
 * TODO: api
 */
bool Device_impl_update_key(Device_impl* dimpl, const char* key);


/**
 * Deinitialises the Device implementation.
 *
 * \param dimpl   The Device implementation -- must not be \c NULL.
 */
void Device_impl_deinit(Device_impl* dimpl);


/**
 * Destroys the Device implementation.
 *
 * \param dimpl   The Device implementation, or \c NULL.
 */
void del_Device_impl(Device_impl* dimpl);


#endif // K_DEVICE_IMPL_H


