

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


#include <init/devices/Device_port_groups.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <player/Work_buffer.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#if 0
static void Device_port_groups_print(const Device_port_groups groups)
{
    fprintf(stdout, "Port groups:");

    for (int i = 0; (i < PORT_GROUPS_MAX) && (groups[i] != 0); ++i)
        fprintf(stdout, " %d", groups[i]);

    fprintf(stdout, "\n");

    return;
}
#endif


void Device_port_groups_init(Device_port_groups groups, int first_group_size, ...)
{
    rassert(groups != NULL);
    rassert(first_group_size >= 0);
    rassert(implies(first_group_size > 0, is_p2(first_group_size)));
    rassert(first_group_size <= WORK_BUFFER_SUB_COUNT_MAX);

    for (int i = 0; i < PORT_GROUPS_MAX; ++i)
        groups[i] = 0;

    va_list args;
    va_start(args, first_group_size);

    groups[0] = (int8_t)first_group_size;

    int size_count = 0;

    if (first_group_size > 0)
    {
        for (size_count = 1; size_count < PORT_GROUPS_MAX; ++size_count)
        {
            const int size = va_arg(args, int);
            rassert(size >= 0);
            rassert(size <= WORK_BUFFER_SUB_COUNT_MAX);
            rassert(implies(size > 0, is_p2(size)));
            groups[size_count] = (int8_t)size;

            if (groups[size_count] == 0)
                break;
        }
    }

    if (size_count == PORT_GROUPS_MAX)
    {
        const int zero = va_arg(args, int);
        rassert(zero == 0);
    }

    va_end(args);

    return;
}


int Device_port_groups_get_alloc_info(
        const Device_port_groups groups, int port, int* sub_index)
{
    rassert(groups != NULL);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    int port_count = 0;
    int group_count = 0;
    for (int i = 0; (i < PORT_GROUPS_MAX) && (groups[i] != 0); ++i)
    {
        const int next_port_count = port_count + groups[i];
        if ((port_count <= port) && (port < next_port_count))
        {
            if (sub_index != NULL)
                *sub_index = port - port_count;
            return i;
        }

        port_count = next_port_count;
        group_count = (i + 1);
    }

    if (sub_index != NULL)
        *sub_index = 0;

    return group_count + (port - port_count);
}


