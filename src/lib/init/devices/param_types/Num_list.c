

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Num_list.h>

#include <debug/assert.h>
#include <memory.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Num_list
{
    int32_t len;
    int32_t res;
    double* nums;
};


static bool Num_list_append(Num_list* nl, double num);


static bool read_num(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);
    rassert(userdata != NULL);

    Num_list* nl = userdata;

    double num = NAN;
    if (!Streader_read_float(sr, &num))
        return false;

    if (!Num_list_append(nl, num))
    {
        del_Num_list(nl);
        Streader_set_memory_error(
                sr, "Could not allocate memory for number list");
        return false;
    }

    return true;
}

Num_list* new_Num_list_from_string(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    if (!Streader_has_data(sr))
        return NULL;

    Num_list* nl = memory_alloc_item(Num_list);
    if (nl == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for number list");
        return NULL;
    }

    nl->len = 0;
    nl->res = 8;
    nl->nums = NULL;

    nl->nums = memory_alloc_items(double, nl->res);
    if (nl->nums == NULL)
    {
        del_Num_list(nl);
        Streader_set_memory_error(
                sr, "Could not allocate memory for number list");
        return NULL;
    }

    if (!Streader_read_list(sr, read_num, nl))
    {
        del_Num_list(nl);
        return NULL;
    }

    return nl;
}


static bool Num_list_append(Num_list* nl, double num)
{
    rassert(nl != NULL);
    rassert(!isnan(num));

    if (nl->len >= nl->res)
    {
        rassert(nl->len == nl->res);

        double* new_nums = memory_realloc_items(double, nl->res * 2, nl->nums);
        if (new_nums == NULL)
            return false;

        nl->nums = new_nums;
        nl->res *= 2;
    }

    nl->nums[nl->len] = num;
    ++nl->len;

    return true;
}


int32_t Num_list_length(const Num_list* nl)
{
    rassert(nl != NULL);
    return nl->len;
}


double Num_list_get_num(const Num_list* nl, int32_t index)
{
    rassert(nl != NULL);
    rassert(index >= 0);
    rassert(index < nl->len);

    return nl->nums[index];
}


void del_Num_list(Num_list* nl)
{
    if (nl == NULL)
        return;

    memory_free(nl->nums);
    memory_free(nl);

    return;
}


