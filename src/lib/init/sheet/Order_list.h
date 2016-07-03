

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ORDER_LIST_H
#define KQT_ORDER_LIST_H


#include <string/Streader.h>

#include <stdbool.h>
#include <stdlib.h>


/**
 * Order list specifies the playback order of Pattern instances.
 */
typedef struct Order_list Order_list;


/**
 * Create a new Order list.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Order list if successful, otherwise \c NULL. \a state
 *           will not contain error if memory allocation failed.
 */
Order_list* new_Order_list(Streader* sr);


/**
 * Return the length of the Order list.
 *
 * \param ol   The Order list -- must not be \c NULL.
 *
 * \return   The length.
 */
int Order_list_get_len(const Order_list* ol);


/**
 * Return a Pattern instance reference from the Order list.
 *
 * \param ol      The Order list -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and
 *                < Order_list_get_len(\a ol).
 *
 * \return   The Pattern instance reference.
 */
Pat_inst_ref* Order_list_get_pat_inst_ref(const Order_list* ol, int index);


/**
 * Check if a Pattern instance reference is in the Order list.
 *
 * The implementation performs a linear search over the Order list.
 *
 * \param ol      The Order list -- must not be \c NULL.
 * \param piref   The Pattern instance reference to look for -- must not be
 *                \c NULL.
 *
 * \return   \c true if \a ol contains \a piref, otherwise \c false.
 */
bool Order_list_contains_pat_inst_ref(const Order_list* ol, const Pat_inst_ref* piref);


/**
 * Destroy an existing Order list.
 *
 * \param ol   The Order list, or \c NULL.
 */
void del_Order_list(Order_list* ol);


#endif // KQT_ORDER_LIST_H


