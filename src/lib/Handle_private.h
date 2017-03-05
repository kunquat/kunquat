

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_HANDLE_PRIVATE_H
#define KQT_HANDLE_PRIVATE_H


#include <kunquat/Handle.h>

#include <Error.h>
#include <init/Module.h>
#include <kunquat/Player.h>
#include <player/Player.h>

#include <stdbool.h>


#define POSITION_LENGTH (64)

#define DEFAULT_BUFFER_SIZE (2048)


typedef enum
{
    ERROR_IMMEDIATE,
    ERROR_VALIDATION,
} Error_delay_type;


typedef struct Handle
{
    bool data_is_valid;
    bool data_is_validated;
    bool update_connections;
    Module* module;
    Error error;
    Error validation_error;
    char position[POSITION_LENGTH];

    Player* player;
    Player* length_counter;
} Handle;


/**
 * Initialise a Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 *
 * \return   \c true if successful. Otherwise, \c false is returned and Handle
 *           error is set to indicate the error.
 */
bool Handle_init(Handle* handle);


/**
 * Set an error message for a Kunquat Handle.
 *
 * The caller should always use one of the macro versions.
 *
 * \param handle    The Kunquat Handle, or \c NULL if not applicable.
 * \param type      The error type -- must be > \c ERROR_NONE and
 *                  < \c ERROR_LAST.
 * \param message   The error message format -- must not be \c NULL. This and
 *                  the extra arguments correspond to the arguments of the
 *                  printf family of functions.
 */
#define Handle_set_error(handle, type, ...) \
    (Handle_set_error_((handle), (type), ERROR_IMMEDIATE, \
                           __FILE__, __LINE__, __func__, __VA_ARGS__))

#define Handle_set_validation_error(handle, type, ...) \
    (Handle_set_error_((handle), (type), ERROR_VALIDATION, \
                           __FILE__, __LINE__, __func__, __VA_ARGS__))

void Handle_set_error_(
        Handle* handle,
        Error_type type,
        Error_delay_type delay_type,
        const char* file,
        int line,
        const char* func,
        const char* message, ...);


/**
 * Copy an error to a Kunquat Handle.
 *
 * \param handle   The Kunquat Handle, or \c NULL if not applicable.
 * \param error    The Error -- must not be \c NULL and
 *                 must have an error set.
 */
void Handle_set_error_from_Error(Handle* handle, const Error* error);


/**
 * Copy a validation error to a Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 * \param error    The Error -- must not be \c NULL and
 *                 must have an error set.
 */
void Handle_set_validation_error_from_Error(Handle* handle, const Error* error);


/**
 * Check the validity of a Kunquat Handle.
 *
 * \param handle   The ID of the supposed Kunquat Handle.
 *
 * \return   \c true if \a handle is a valid Kunquat Handle, otherwise
 *           \c false.
 */
bool kqt_Handle_is_valid(kqt_Handle handle);


/**
 * Get the number of mixing buffers in the Kunquat Handle.
 *
 * \param handle   The Handle -- must not be \c NULL.
 *
 * \return   The number of buffers, or \c 0 if \a handle == \c NULL.
 *           \c 1 indicates monophonic audio and \c 2 indicates stereo audio.
 */
int Handle_get_buffer_count(Handle* handle);


#define check_handle(id, ret)                            \
    if (true)                                            \
    {                                                    \
        if (!kqt_Handle_is_valid((id)))                  \
        {                                                \
            Handle_set_error(NULL, ERROR_ARGUMENT,       \
                    "Invalid Kunquat Handle: %d", (id)); \
            return (ret);                                \
        }                                                \
    } else ignore(0)

#define check_handle_void(id)                            \
    if (true)                                            \
    {                                                    \
        if (!kqt_Handle_is_valid((id)))                  \
        {                                                \
            Handle_set_error(NULL, ERROR_ARGUMENT,       \
                    "Invalid Kunquat Handle: %d", (id)); \
            return;                                      \
        }                                                \
    } else ignore(0)


Handle* get_handle(kqt_Handle id);


#define check_data_is_valid(handle, ret) \
    if (true)                            \
    {                                    \
        if (!handle->data_is_valid)      \
            return (ret);                \
    } else ignore(0)

#define check_data_is_valid_void(handle) \
    if (true)                            \
    {                                    \
        if (!handle->data_is_valid)      \
            return;                      \
    } else ignore(0)


#define check_data_is_validated(handle, ret)                          \
    if (true)                                                         \
    {                                                                 \
        if (!handle->data_is_validated)                               \
        {                                                             \
            Handle_set_error((handle), ERROR_ARGUMENT,                \
                    "Data is not validated (call kqt_Handle_validate" \
                    " before calling this function)");                \
            return (ret);                                             \
        }                                                             \
    } else ignore(0)

#define check_data_is_validated_void(handle)                          \
    if (true)                                                         \
    {                                                                 \
        if (!handle->data_is_validated)                               \
        {                                                             \
            Handle_set_error((handle), ERROR_ARGUMENT,                \
                    "Data is not validated (call kqt_Handle_validate" \
                    " before calling this function)");                \
            return;                                                   \
        }                                                             \
    } else ignore(0)


bool key_is_valid(Handle* handle, const char* key);


#define check_key(handle, key, ret)         \
    if (true)                               \
    {                                       \
        rassert((handle) != NULL);          \
        if (!key_is_valid((handle), (key))) \
            return (ret);                   \
    }                                       \
    else ignore(0)


/**
 * Refresh environment states of all the Players in the Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Handle_refresh_env_states(Handle* handle);


/**
 * Get the module associated with the Handle.
 *
 * \param handle   The Handle -- must not be \c NULL.
 *
 * \return   The Module.
 */
Module* Handle_get_module(Handle* handle);


#endif // KQT_HANDLE_PRIVATE_H


