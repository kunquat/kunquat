

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PORT_TYPE_H
#define KQT_PORT_TYPE_H


typedef enum
{
    DEVICE_PORT_TYPE_RECEIVE = 0,
    DEVICE_PORT_TYPE_SEND,
    DEVICE_PORT_TYPES             ///< Sentinel -- not a valid type.
} Device_port_type;


#endif // KQT_PORT_TYPE_H


