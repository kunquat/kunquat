

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


#include <Pat_inst_ref.h>

#include <debug/assert.h>

#include <stdbool.h>


static bool Pat_inst_ref_is_valid(const Pat_inst_ref* p)
{
    const bool is_normal =
        (p != NULL) &&
        (p->pat >= 0) &&
        (p->pat < KQT_PATTERNS_MAX) &&
        (p->inst >= 0) &&
        (p->inst < KQT_PAT_INSTANCES_MAX);

    return is_normal || (p->pat == -1 && p->inst == -1);
}


int Pat_inst_ref_cmp(const Pat_inst_ref* p1, const Pat_inst_ref* p2)
{
    rassert(Pat_inst_ref_is_valid(p1));
    rassert(Pat_inst_ref_is_valid(p2));

    if (p1->pat < p2->pat)
        return -1;
    else if (p1->pat > p2->pat)
        return 1;
    else if (p1->inst < p2->inst)
        return -1;
    else if (p1->inst > p2->inst)
        return 1;
    return 0;
}


