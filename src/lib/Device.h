

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DEVICE_H
#define K_DEVICE_H


#include <stdbool.h>
#include <stdint.h>

#include <Audio_buffer.h>
#include <frame.h>
#include <kunquat/limits.h>


typedef enum
{
    DEVICE_PORT_TYPE_RECEIVE = 0,
    DEVICE_PORT_TYPE_SEND,
    DEVICE_PORT_TYPES             ///< Sentinel -- not a valid type.
} Device_port_type;


typedef struct Device
{
    uint32_t buffer_size;
    bool reg[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Audio_buffer* buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Audio_buffer* direct_send[KQT_DEVICE_PORTS_MAX];
} Device;


/**
 * Initialises the Device.
 *
 * \param device        The Device -- must not be \c NULL.
 * \param buffer_size   The current buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init(Device* device, uint32_t buffer_size);


/**
 * Registers a new port for the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. If the port is already
 *                 registered, the function does nothing.
 */
void Device_register_port(Device* device, Device_port_type type, int port);


/**
 * Unregisters a port of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. If the port is not registered,
 *                 the function does nothing.
 */
void Device_unregister_port(Device* device, Device_port_type type, int port);


/**
 * Sets a direct send buffer for the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param port     The send port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 * \param buffer   The Audio buffer, or \c NULL.
 */
void Device_set_direct_send(Device* device,
                            int port,
                            Audio_buffer* buffer);


/**
 * Initialises the internal buffers of the Device.
 *
 * This function should be called after each time the Device connection graph
 * is changed.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init_buffers(Device* device);


/**
 * Resizes the buffers in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param size     The new buffer size -- must be > \c 0 and <=
 *                 \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_resize_buffers(Device* device, uint32_t size);


/**
 * Retrieves an Audio buffer from the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   The buffer, or \c NULL if the port is not registered.
 */
Audio_buffer* Device_get_buffer(Device* device,
                                Device_port_type type,
                                int port);


/**
 * Clears all the Audio buffers of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 */
void Device_clear_buffers(Device* device);


/**
 * Uninitialises the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 */
void Device_uninit(Device* device);


#endif // K_DEVICE_H


