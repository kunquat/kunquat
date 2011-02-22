

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <stdio.h>

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
    uint32_t mix_rate;
    uint32_t buffer_size;
    bool (*set_mix_rate)(struct Device*, uint32_t);
    bool (*set_buffer_size)(struct Device*, uint32_t);
    void (*reset)(struct Device*);
    bool (*sync)(struct Device*);
    bool (*update_key)(struct Device*, const char*);
    void (*process)(struct Device*, uint32_t, uint32_t, uint32_t, double);
    bool reg[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Audio_buffer* buffers[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
    Audio_buffer* direct_receive[KQT_DEVICE_PORTS_MAX];
    Audio_buffer* direct_send[KQT_DEVICE_PORTS_MAX];
} Device;


/**
 * Initialises the Device.
 *
 * \param device        The Device -- must not be \c NULL.
 * \param buffer_size   The current buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The current mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init(Device* device, uint32_t buffer_size, uint32_t mix_rate);


/**
 * Sets the function for changing the mixing rate of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param changer   The change function, or \c NULL.
 */
void Device_set_mix_rate_changer(Device* device,
                                 bool (*changer)(Device*, uint32_t));


/**
 * Sets the function for changing the buffer size of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param changer   The change function, or \c NULL.
 */
void Device_set_buffer_size_changer(Device* device,
                                    bool (*changer)(Device*, uint32_t));


/**
 * Sets the playback reset function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param reset    The reset function -- must not be \c NULL.
 */
void Device_set_reset(Device* device, void (*reset)(Device*));


/**
 * Sets the synchronisation function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param sync     The synchronisation function -- must not be \c NULL.
 */
void Device_set_sync(Device* device, bool (*sync)(Device*));


/**
 * Sets the update notification function of the Device.
 *
 * \param device       The Device -- must not be \c NULL.
 * \param update_key   The update notification function
 *                     -- must not be \c NULL.
 */
void Device_set_update_key(Device* device,
                           bool (*update_key)(struct Device*, const char*));


/**
 * Sets the process function of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param process   The process function -- must not be \c NULL.
 */
void Device_set_process(Device* device,
                        void (*process)(Device*, uint32_t, uint32_t,
                                                 uint32_t, double));


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
 * Finds out whether a port is registered in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if the port is registered, otherwise \c false.
 */
bool Device_port_is_registered(Device* device,
                               Device_port_type type,
                               int port);


/**
 * Initialises a buffer for the Device.
 *
 * Initialising a send buffer will replace a direct send buffer if one exists
 * for the same send port.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. If the port is not registered,
 *                 the function does nothing and succeeds.
 *
 * \param   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init_buffer(Device* device, Device_port_type type, int port);


/**
 * Sets a direct receive buffer for the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param port     The receive port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX. Also, the send port with this
 *                 number must contain a buffer.
 */
void Device_set_direct_receive(Device* device, int port);


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
 * Removes all direct buffers from the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 */
void Device_remove_direct_buffers(Device* device);


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
 * Initialises the internal buffers of the Device.
 *
 * This function should be called after each time the Device connection graph
 * is changed.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
//bool Device_init_buffers(Device* device);


/**
 * Sets the mixing rate of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param rate     The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_mix_rate(Device* device, uint32_t rate);


/**
 * Gets the mixing rate of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The mixing rate.
 */
uint32_t Device_get_mix_rate(Device* device);


/**
 * Resizes the buffers in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param size     The new buffer size -- must be > \c 0 and <=
 *                 \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_buffer_size(Device* device, uint32_t size);


/**
 * Gets the buffer size of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The buffer size.
 */
uint32_t Device_get_buffer_size(Device* device);


/**
 * Clears all the Audio buffers owned by the Device.
 *
 * This function does not clear direct send buffers.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param start    The first frame to be cleared -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be cleared -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be cleared.
 */
void Device_clear_buffers(Device* device, uint32_t start, uint32_t until);


/**
 * Resets the internal playback state of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 */
void Device_reset(Device* device);


/**
 * Synchronises the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_sync(Device* device);


/**
 * Notifies the Device of a key change and updates the internal state.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param key      The key that changed -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_update_key(Device* device, const char* key);


/**
 * Processes audio in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param start    The first frame to be processed -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be processed -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be cleared.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Device_process(Device* device,
                    uint32_t start,
                    uint32_t until,
                    uint32_t freq,
                    double tempo);


/**
 * Prints a textual description of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param out      The output file -- must not be \c NULL.
 */
void Device_print(Device* device, FILE* out);


/**
 * Uninitialises the Device.
 *
 * \param device   The Device, or \c NULL.
 */
void Device_uninit(Device* device);


#endif // K_DEVICE_H


