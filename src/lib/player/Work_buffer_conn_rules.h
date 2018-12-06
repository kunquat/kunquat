

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


#ifndef KQT_WORK_BUFFER_CONNECTION_H
#define KQT_WORK_BUFFER_CONNECTION_H


#include <decl.h>
#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Work_buffer_connection
{
    uint8_t recv_sub_index : 4;
    uint8_t send_sub_index : 4;
} Work_buffer_connection;


struct Work_buffer_conn_rules
{
    Work_buffer* receiver;
    const Work_buffer* sender;
    int8_t length;
    Work_buffer_connection conns[WORK_BUFFER_SUB_COUNT_MAX];
};


#define WORK_BUFFER_CONN_RULES_AUTO \
    (&(Work_buffer_conn_rules){ .receiver = NULL, .sender = NULL, .length = 0 })


Work_buffer_conn_rules* Work_buffer_conn_rules_init(
        Work_buffer_conn_rules* rules,
        Work_buffer* receiver,
        int receiver_sub_index,
        const Work_buffer* sender,
        int sender_sub_index);


bool Work_buffer_conn_rules_try_merge(
        Work_buffer_conn_rules* result,
        const Work_buffer_conn_rules* rules1,
        const Work_buffer_conn_rules* rules2);


//void Work_buffer_conn_rules_copy(
//        const Work_buffer_conn_rules* rules, int32_t frame_count);


void Work_buffer_conn_rules_mix(
        const Work_buffer_conn_rules* rules, int32_t frame_count);


#endif // KQT_WORK_BUFFER_CONNECTION_H


