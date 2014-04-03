

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_ORDER_LIST_H
#define K_ORDER_LIST_H


#include <stdlib.h>

#include <Streader.h>


/**
 * Order list specifies the playback order of Pattern instances.
 */
typedef struct Order_list Order_list;


/**
 * Creates a new Order list.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Order list if successful, otherwise \c NULL. \a state
 *           will not contain error if memory allocation failed.
 */
Order_list* new_Order_list(Streader* sr);


/**
 * Returns the length of the Order list.
 *
 * \param ol   The Order list -- must not be \c NULL.
 *
 * \return   The length.
 */
size_t Order_list_get_len(const Order_list* ol);


/**
 * Returns a Pattern instance reference from the Order list.
 *
 * \param ol      The Order list -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and
 *                < Order_list_get_len(\a ol).
 *
 * \return   The Pattern instance reference.
 */
Pat_inst_ref* Order_list_get_pat_inst_ref(const Order_list* ol, size_t index);


/**
 * Destroys an existing Order list.
 *
 * \param ol   The Order list, or \c NULL.
 */
void del_Order_list(Order_list* ol);


#endif // K_ORDER_LIST_H


