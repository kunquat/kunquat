

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

#include <Etable.h>
#include <Order.h>

#include <xmemory.h>


typedef struct Subsong
{
    int res;
    int16_t* pats;
} Subsong;


struct Order
{
    Etable* subs;
};


/**
 * Creates a new Subsong.
 *
 * \return   The new Subsong if successful, or \c NULL if memory allocation
 *           failed.
 */
static Subsong* new_Subsong(void);


/**
 * Sets the pattern for the specified Subsong position.
 *
 * \param subsong   The Subsong -- must not be \c NULL.
 * \param index     The index -- must be >= \c 0 and < \c ORDERS_MAX.
 * \param pat       The pattern number -- must be >= \c 0 or ORDER_NONE.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Subsong_set(Subsong* ss, int index, int16_t pat);


/**
 * Gets the pattern from the specified Subsong position.
 *
 * \param subsong   The Subsong -- must not be \c NULL.
 * \param index     The index -- must be >= \c 0 and < \c ORDERS_MAX.
 *
 * \return   The pattern number if one exists, otherwise ORDER_NONE.
 */
static int16_t Subsong_get(Subsong* ss, int index);


/**
 * Clears the Subsong.
 *
 * \param subsong   The Subsong -- must not be \c NULL.
 */
// static void Subsong_clear(Subsong* ss);


/**
 * Destroys an existing Subsong.
 *
 * \param subsong   The Subsong -- must not be \c NULL.
 */
static void del_Subsong(Subsong* ss);


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


static Subsong* new_Subsong(void)
{
    Subsong* ss = xalloc(Subsong);
    if (ss == NULL)
    {
        return NULL;
    }
    ss->res = 8;
    ss->pats = xnalloc(int16_t, ss->res);
    if (ss->pats == NULL)
    {
        xfree(ss);
        return NULL;
    }
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = ORDER_NONE;
    }
    return ss;
}


static bool Subsong_set(Subsong* ss, int index, int16_t pat)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < ORDERS_MAX);
    assert(pat >= 0 || pat == ORDER_NONE);
    if (index >= ss->res)
    {
        int new_res = ss->res << 1;
        if (index >= new_res)
        {
            new_res = index + 1;
        }
        int16_t* new_pats = xrealloc(int16_t, new_res, ss->pats);
        if (new_pats == NULL)
        {
            return false;
        }
        ss->pats = new_pats;
        for (int i = ss->res; i < new_res; ++i)
        {
            ss->pats[i] = ORDER_NONE;
        }
        ss->res = new_res;
    }
    ss->pats[index] = pat;
    return true;
}


static int16_t Subsong_get(Subsong* ss, int index)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < ORDERS_MAX);
    if (index >= ss->res)
    {
        return ORDER_NONE;
    }
    return ss->pats[index];
}


#if 0
static void Subsong_clear(Subsong* ss)
{
    assert(ss != NULL);
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = ORDER_NONE;
    }
    return;
}
#endif


static void del_Subsong(Subsong* ss)
{
    assert(ss != NULL);
    xfree(ss->pats);
    xfree(ss);
    return;
}


