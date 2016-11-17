

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


#ifndef KQT_DEVICE_NODE_STATE_H
#define KQT_DEVICE_NODE_STATE_H


/**
 * The state of a Device node during a graph search. These are sometimes
 * referred to as the colours white, grey and black.
 */
typedef enum
{
    DEVICE_NODE_STATE_NEW = 0,
    DEVICE_NODE_STATE_REACHED,
    DEVICE_NODE_STATE_VISITED,
    DEVICE_NODE_STATE_COUNT
} Device_node_state;


#endif // KQT_DEVICE_NODE_STATE_H


