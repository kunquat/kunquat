

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PORT_GROUPS_H
#define KQT_PORT_GROUPS_H


#include <stdint.h>
#include <stdlib.h>


#define PORT_GROUPS_MAX 4
typedef int8_t Device_port_groups[PORT_GROUPS_MAX];


/**
 * Initialise Device port groups.
 *
 * \param groups             The Device port groups -- must not be \c NULL.
 * \param first_group_size   The size of the first group -- must be >= \c 0,
 *                           < \c WORK_BUFFER_SUB_COUNT_MAX and a power of two if
 *                           > \c 0. The following arguments must be of the same
 *                           format, and \c 0 is treated as a list terminator.
 *                           The number of actual group sizes must not exceed
 *                           \a PORT_GROUPS_MAX. The list of actual groups sizes
 *                           must always be followed by a list terminator.
 */
void Device_port_groups_init(Device_port_groups groups, int first_group_size, ...);


/**
 * Get port allocation information from the Device port groups.
 *
 * \param groups      The Device port groups -- must not be \c NULL.
 * \param port        The Device port number -- must be >= \c 0 and
 *                    < \c KQT_DEVICE_PORTS_MAX.
 * \param sub_index   Destination address where the buffer area index associated
 *                    with \a port is stored. This may also be \c NULL.
 *
 * \return   The Work buffer index associated with \a port.
 */
int Device_port_groups_get_alloc_info(
        const Device_port_groups groups, int port, int* sub_index);


#endif // KQT_PORT_GROUPS_H


