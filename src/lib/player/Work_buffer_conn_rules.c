

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
#include <stdio.h>
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


static void Work_buffer_conn_rules_copy_rules(
        Work_buffer_conn_rules* dest, const Work_buffer_conn_rules* src)
{
    rassert(dest != NULL);
    rassert(src != NULL);

    if (dest == src)
        return;

    memcpy(dest, src, sizeof(Work_buffer_conn_rules));

    return;
}


#if 0
static void print_rules(const Work_buffer_conn_rules* rules)
{
    rassert(rules != NULL);

    FILE* out = stdout;
    fprintf(out, "(%p -> %p, %d: [",
            (const void*)rules->sender, (const void*)rules->receiver, (int)rules->length);
    for (int i = 0; i < rules->length; ++i)
    {
        const Work_buffer_connection* conn = &rules->conns[i];
        if (i != 0)
            fprintf(out, ", ");
        fprintf(out, "%d -> %d", conn->send_sub_index, conn->recv_sub_index);
    }
    fprintf(out, "])");

    return;
}
#endif


bool Work_buffer_conn_rules_try_merge(
        Work_buffer_conn_rules* result,
        const Work_buffer_conn_rules* rules1,
        const Work_buffer_conn_rules* rules2)
{
    rassert(result != NULL);
    rassert(rules1 != NULL);
    rassert(rules2 != NULL);
    rassert(rules2 != rules1);

#if 0
    fprintf(stdout, "Try to merge ");
    print_rules(rules1);
    fprintf(stdout, " with ");
    print_rules(rules2);
    fprintf(stdout, "\n");
    fflush(stdout);
#endif

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
                    //fprintf(stdout, "Direct 2-2 connection\n");
                    //fflush(stdout);
                    Work_buffer_conn_rules_copy_rules(result, rules1);
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


static bool mapping_is_direct(const Work_buffer_conn_rules* rules, uint8_t* conn_mask)
{
    rassert(rules != NULL);
    rassert(conn_mask != NULL);

    const int recv_sub_count = Work_buffer_get_sub_count(rules->receiver);
    const int send_sub_count = Work_buffer_get_sub_count(rules->sender);

    if (recv_sub_count != send_sub_count)
        return false;

    uint8_t mask = 0;

    for (int i = 0; i < rules->length; ++i)
    {
        if (!Work_buffer_connection_is_direct(&rules->conns[i]))
            return false;

        mask |= (uint8_t)(1 << rules->conns[i].recv_sub_index);
    }

    *conn_mask = mask;

    return true;
}


#if 0
void Work_buffer_conn_rules_copy(
        const Work_buffer_conn_rules* rules, int32_t frame_count)
{
    rassert(rules != NULL);
    rassert(frame_count > 0);

    uint8_t conn_mask = 0;

    if (mapping_is_direct(rules, &conn_mask))
    {
        Work_buffer_copy_all(rules->receiver, rules->sender, 0, frame_count, conn_mask);
        return;
    }

    for (int i = 0; i < rules->length; ++i)
        Work_buffer_copy(
                rules->receiver,
                rules->conns[i].recv_sub_index,
                rules->sender,
                rules->conns[i].send_sub_index,
                0,
                frame_count);

    return;
}
#endif


void Work_buffer_conn_rules_mix(
        const Work_buffer_conn_rules* rules, int32_t frame_count)
{
    rassert(rules != NULL);
    rassert(frame_count >= 0);

    if (frame_count == 0)
        return;

    uint8_t conn_mask = 0;

    if (mapping_is_direct(rules, &conn_mask))
    {
        Work_buffer_mix_all(rules->receiver, rules->sender, 0, frame_count, conn_mask);
        return;
    }

    for (int i = 0; i < rules->length; ++i)
        Work_buffer_mix(
                rules->receiver,
                rules->conns[i].recv_sub_index,
                rules->sender,
                rules->conns[i].send_sub_index,
                0,
                frame_count);

    return;
}


