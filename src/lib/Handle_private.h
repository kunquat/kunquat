

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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


#include <stdbool.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>

#include <Module.h>
#include <player/Player.h>


#define KQT_HANDLE_ERROR_LENGTH (512)

#define POSITION_LENGTH (64)

#define DEFAULT_BUFFER_SIZE (2048)


typedef enum
{
    ERROR_NONE = 0,
    ERROR_ARGUMENT, // libkunquat function called with an invalid argument
    ERROR_FORMAT,   // input file structure error
    ERROR_MEMORY,   // out of memory
    ERROR_RESOURCE, // resource (external lib or fs) failure
    ERROR_LAST      // sentinel value
} Error_type;


typedef enum
{
    ERROR_IMMEDIATE,
    ERROR_VALIDATION,
} Error_delay_type;


struct kqt_Handle
{
    bool data_is_valid;
    bool data_is_validated;
    Module* module;
    int (*set_data)(kqt_Handle* handle, const char* key, void* data, long length);
    void (*destroy)(struct kqt_Handle* handle);
    char error[KQT_HANDLE_ERROR_LENGTH];
    char validation_error[KQT_HANDLE_ERROR_LENGTH];
    char position[POSITION_LENGTH];

    Player* player;
    Player* length_counter;
};


/**
 * Initialises a Kunquat Handle.
 *
 * \param handle        The Kunquat Handle -- must not be \c NULL.
 * \param buffer_size   The size of the mixing buffers -- must be positive.
 *
 * \return   \c true if successful. Otherwise, \c false is returned and Handle
 *           error is set to indicate the error.
 */
bool kqt_Handle_init(kqt_Handle* handle, long buffer_size);


/**
 * Sets an error message for a Kunquat Handle.
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
#define kqt_Handle_set_error(handle, type, ...) \
    (kqt_Handle_set_error_((handle), (type), ERROR_IMMEDIATE, \
                           __FILE__, __LINE__, __func__, __VA_ARGS__))

#define kqt_Handle_set_validation_error(handle, type, ...) \
    (kqt_Handle_set_error_((handle), (type), ERROR_VALIDATION, \
                           __FILE__, __LINE__, __func__, __VA_ARGS__))

void kqt_Handle_set_error_(kqt_Handle* handle,
                           Error_type type,
                           Error_delay_type delay_type,
                           const char* file,
                           int line,
                           const char* func,
                           const char* message, ...);


/**
 * Resets the playback pointer of the Kunquat Handle.
 *
 * \param handle   The Kunquat Handle -- must not be \c NULL.
 */
void kqt_Handle_stop(kqt_Handle* handle);


/**
 * Checks the validity of a Kunquat Handle.
 *
 * \param handle   The pointer of the supposed Kunquat Handle.
 *
 * \return   \c true if \a handle is a valid Kunquat Handle, otherwise
 *           \c false.
 */
bool handle_is_valid(kqt_Handle* handle);


/**
 * Gets the number of mixing buffers in the Kunquat Handle.
 *
 * \param handle   The Handle -- must not be \c NULL.
 *
 * \return   The number of buffers, or \c 0 if \a handle == \c NULL.
 *           \c 1 indicates monophonic audio and \c 2 indicates stereo audio.
 */
int kqt_Handle_get_buffer_count(kqt_Handle* handle);


#define check_handle(handle, ret)                                   \
    if (true)                                                       \
    {                                                               \
        if (!handle_is_valid((handle)))                             \
        {                                                           \
            kqt_Handle_set_error(NULL, ERROR_ARGUMENT,              \
                    "Invalid Kunquat Handle: %p", (void*)(handle)); \
            return (ret);                                           \
        }                                                           \
    } else (void)0

#define check_handle_void(handle)                                   \
    if (true)                                                       \
    {                                                               \
        if (!handle_is_valid((handle)))                             \
        {                                                           \
            kqt_Handle_set_error(NULL, ERROR_ARGUMENT,              \
                    "Invalid Kunquat Handle: %p", (void*)(handle)); \
            return;                                                 \
        }                                                           \
    } else (void)0


#define check_data_is_valid(handle, ret) \
    if (true)                            \
    {                                    \
        if (!handle->data_is_valid)      \
        {                                \
            return (ret);                \
        }                                \
    } else (void)0

#define check_data_is_valid_void(handle) \
    if (true)                            \
    {                                    \
        if (!handle->data_is_valid)      \
        {                                \
            return;                      \
        }                                \
    } else (void)0


#define check_data_is_validated(handle, ret)                          \
    if (true)                                                         \
    {                                                                 \
        if (!handle->data_is_validated)                               \
        {                                                             \
            kqt_Handle_set_error((handle), ERROR_ARGUMENT,            \
                    "Data is not validated (call kqt_Handle_validate" \
                    " before calling this function)");                \
            return (ret);                                             \
        }                                                             \
    } else (void)0

#define check_data_is_validated_void(handle)                          \
    if (true)                                                         \
    {                                                                 \
        if (!handle->data_is_validated)                               \
        {                                                             \
            kqt_Handle_set_error((handle), ERROR_ARGUMENT,            \
                    "Data is not validated (call kqt_Handle_validate" \
                    " before calling this function)");                \
            return;                                                   \
        }                                                             \
    } else (void)0


bool key_is_valid(kqt_Handle* handle, const char* key);


#define check_key(handle, key, ret)         \
    if (true)                               \
    {                                       \
        assert((handle) != NULL);           \
        if (!key_is_valid((handle), (key))) \
        {                                   \
            return (ret);                   \
        }                                   \
    }                                       \
    else (void)0


/**
 * Gets the module associated with the Handle.
 *
 * \param handle   The Handle -- must not be \c NULL.
 *
 * \return   The Module.
 */
Module* Handle_get_module(kqt_Handle* handle);


#endif // KQT_HANDLE_PRIVATE_H


