

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


#include <player/Work_buffer_conn_rules.h>

#include <debug/assert.h>
#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static void Work_buffer_connection_init(Work_buffer_connection* conn)
{
    rassert(conn != NULL);

    conn->recv_sub_index = 0;
    conn->send_sub_index = 0;

    return;
}


static bool Work_buffer_connection_is_direct(const Work_buffer_connection* conn)
{
    rassert(conn != NULL);
    return (conn->recv_sub_index == conn->send_sub_index);
}


Work_buffer_conn_rules* Work_buffer_conn_rules_init(
        Work_buffer_conn_rules* rules,
        Work_buffer* receiver,
        int receiver_sub_index,
        const Work_buffer* sender,
        int sender_sub_index)
{
    rassert(rules != NULL);
    rassert(receiver != NULL);
    rassert(receiver_sub_index >= 0);
    rassert(receiver_sub_index < Work_buffer_get_sub_count(receiver));
    rassert(sender != NULL);
    rassert(sender != receiver);
    rassert(sender_sub_index >= 0);
    rassert(sender_sub_index < Work_buffer_get_sub_count(sender));

    for (int i = 0; i < WORK_BUFFER_SUB_COUNT_MAX; ++i)
        Work_buffer_connection_init(&rules->conns[i]);

    rules->receiver = receiver;
    rules->sender = sender;
    rules->length = 1;
    rules->conns[0].recv_sub_index = (uint8_t)receiver_sub_index & 0x7;
    rules->conns[0].send_sub_index = (uint8_t)sender_sub_index & 0x7;

    return rules;
}


static void Work_buffer_conn_rules_copy(
        Work_buffer_conn_rules* dest, const Work_buffer_conn_rules* src)
{
    rassert(dest != NULL);
    rassert(src != NULL);

    if (dest == src)
        return;

    memcpy(dest, src, sizeof(Work_buffer_conn_rules));

    return;
}


bool Work_buffer_conn_rules_try_merge(
        Work_buffer_conn_rules* result,
        const Work_buffer_conn_rules* rules1,
        const Work_buffer_conn_rules* rules2)
{
    rassert(rules1 != NULL);
    rassert(rules2 != NULL);

    if ((rules1->receiver != rules2->receiver) ||
            (rules1->sender != rules2->sender))
        return false;

    if ((Work_buffer_get_sub_count(rules1->receiver) == 2) &&
            (Work_buffer_get_sub_count(rules1->sender) == 2))
    {
        if ((rules1->length == 1) && (rules2->length == 1))
        {
            if (Work_buffer_connection_is_direct(&rules1->conns[0]) &&
                    Work_buffer_connection_is_direct(&rules2->conns[0]))
            {
                const uint8_t index1 = rules1->conns[0].recv_sub_index;
                const uint8_t index2 = rules2->conns[0].recv_sub_index;
                if ((index1 == 0 && index2 == 1) || (index1 == 1 && index2 == 0))
                {
                    Work_buffer_conn_rules_copy(result, rules1);
                    result->length = 2;
                    result->conns[0].recv_sub_index = 0;
                    result->conns[0].send_sub_index = 0;
                    result->conns[1].recv_sub_index = 1;
                    result->conns[1].send_sub_index = 1;

                    return true;
                }
            }
        }
    }

    return false;
}


void Work_buffer_conn_rules_mix(
        const Work_buffer_conn_rules* rules, int32_t buf_start, int32_t buf_stop)
{
    rassert(rules != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop > buf_start);

    const int recv_sub_count = Work_buffer_get_sub_count(rules->receiver);
    const int send_sub_count = Work_buffer_get_sub_count(rules->sender);

    if ((rules->length == recv_sub_count) && (recv_sub_count == send_sub_count))
    {
        bool all_direct = true;
        for (int i = 0; i < rules->length; ++i)
        {
            if (!Work_buffer_connection_is_direct(&rules->conns[i]))
            {
                all_direct = false;
                break;
            }
        }

        if (all_direct)
        {
            Work_buffer_mix_all(rules->receiver, rules->sender, buf_start, buf_stop);
            return;
        }
    }

    for (int i = 0; i < rules->length; ++i)
        Work_buffer_mix(
                rules->receiver,
                rules->conns[i].recv_sub_index,
                rules->sender,
                rules->conns[i].send_sub_index,
                buf_start,
                buf_stop);

    return;
}


