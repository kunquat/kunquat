

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <File_base.h>
#include <Num_list.h>
#include <xassert.h>
#include <xmemory.h>


struct Num_list
{
    int32_t len;
    int32_t res;
    double* nums;
};


static bool Num_list_append(Num_list* nl, double num);


Num_list* new_Num_list_from_string(char* str, Read_state* state)
{
    if (state->error)
    {
        return NULL;
    }
    Num_list* nl = xalloc(Num_list);
    if (nl == NULL)
    {
        return NULL;
    }
    nl->len = 0;
    nl->res = 8;
    nl->nums = NULL;
    nl->nums = xnalloc(double, nl->res);
    if (nl->nums == NULL)
    {
        del_Num_list(nl);
        return NULL;
    }
    if (str == NULL)
    {
        return nl;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Num_list(nl);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return nl;
    }
    Read_state_clear_error(state);
    bool expect_num = true;
    while (expect_num)
    {
        double num = NAN;
        str = read_double(str, &num, state);
        if (state->error)
        {
            del_Num_list(nl);
            return NULL;
        }
        if (!Num_list_append(nl, num))
        {
            del_Num_list(nl);
            return NULL;
        }
        check_next(str, state, expect_num);
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_Num_list(nl);
        return NULL;
    }
    return nl;
}


static bool Num_list_append(Num_list* nl, double num)
{
    assert(nl != NULL);
    assert(!isnan(num));
    if (nl->len >= nl->res)
    {
        assert(nl->len == nl->res);
        double* new_nums = xrealloc(double, nl->res * 2, nl->nums);
        if (new_nums == NULL)
        {
            return false;
        }
        nl->nums = new_nums;
        nl->res *= 2;
    }
    nl->nums[nl->len] = num;
    ++nl->len;
    return true;
}


int32_t Num_list_length(Num_list* nl)
{
    assert(nl != NULL);
    return nl->len;
}


double Num_list_get_num(Num_list* nl, int32_t index)
{
    assert(nl != NULL);
    assert(index >= 0);
    assert(index < nl->len);
    return nl->nums[index];
}


void del_Num_list(Num_list* nl)
{
    if (nl == NULL)
    {
        return;
    }
    xfree(nl->nums);
    xfree(nl);
    return;
}


