

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>

#include <Subsong.h>
#include <Etable.h>
#include <Order.h>

#include <xmemory.h>


struct Order
{
    Etable* subs;
};


Order* new_Order(void)
{
    Order* order = xalloc(Order);
    if (order == NULL)
    {
        return NULL;
    }
    order->subs = new_Etable(SUBSONGS_MAX, (void(*)(void*))del_Subsong);
    if (order->subs == NULL)
    {
        xfree(order);
        return NULL;
    }
    return order;
}


bool Order_read(Order* order, File_tree* tree, Read_state* state)
{
    assert(order != NULL);
    assert(tree != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    if (!File_tree_is_dir(tree))
    {
        Read_state_set_error(state, "Subsong collection is not a directory");
        return false;
    }
    for (int i = 0; i < SUBSONGS_MAX; ++i)
    {
        char dir_name[] = "subsong_xx";
        snprintf(dir_name, 11, "subsong_%02x", i);
        File_tree* subsong_tree = File_tree_get_child(tree, dir_name);
        if (subsong_tree != NULL)
        {
            Read_state_init(state, File_tree_get_path(tree));
            if (!Order_set(order, i, 0, 0))
            {
                Read_state_set_error(state,
                         "Couldn't allocate memory for subsong %02x", i);
                return false;
            }
            Subsong* ss = Etable_get(order->subs, i);
            assert(ss != NULL);
            Subsong_read(ss, subsong_tree, state);
            if (state->error)
            {
                return false;
            }
        }
    }
    return true;
}


bool Order_set(Order* order, uint16_t subsong, uint16_t index, int16_t pat)
{
    assert(order != NULL);
    assert(subsong < SUBSONGS_MAX);
    assert(index < ORDERS_MAX);
    assert(pat >= 0 || pat == ORDER_NONE);
    bool ss_is_new = false;
    Subsong* ss = Etable_get(order->subs, subsong);
    if (ss == NULL)
    {
        if (pat == ORDER_NONE)
        {
            return true;
        }
        ss_is_new = true;
        ss = new_Subsong();
        if (ss == NULL)
        {
            return false;
        }
        if (!Etable_set(order->subs, subsong, ss))
        {
            del_Subsong(ss);
            return false;
        }
    }
    if (pat == ORDER_NONE && Subsong_get(ss, index) == ORDER_NONE)
    {
        return true;
    }
    if (!Subsong_set(ss, index, pat))
    {
        if (ss_is_new)
        {
            Etable_remove(order->subs, subsong);
        }
        return false;
    }
    return true;
}


int16_t Order_get(Order* order, uint16_t subsong, uint16_t index)
{
    assert(order != NULL);
    assert(subsong < SUBSONGS_MAX);
    assert(index < ORDERS_MAX);
    Subsong* ss = Etable_get(order->subs, subsong);
    if (ss == NULL)
    {
        return ORDER_NONE;
    }
    return Subsong_get(ss, index);
}


Subsong* Order_get_subsong(Order* order, uint16_t subsong)
{
    assert(order != NULL);
    assert(subsong < SUBSONGS_MAX);
    return Etable_get(order->subs, subsong);
}


bool Order_is_empty(Order* order, uint16_t subsong)
{
    assert(order != NULL);
    assert(subsong < SUBSONGS_MAX);
    Subsong* ss = Etable_get(order->subs, subsong);
    if (ss == NULL)
    {
        return true;
    }
    for (int i = 0; i < ss->res; ++i)
    {
        if (ss->pats[i] != ORDER_NONE)
        {
            return false;
        }
    }
    return true;
}


void del_Order(Order* order)
{
    assert(order != NULL);
    del_Etable(order->subs);
    xfree(order);
    return;
}


