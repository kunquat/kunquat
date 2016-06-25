

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Active_jumps.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdlib.h>


struct Active_jumps
{
    AAtree* jumps;
    size_t use_count; //< for debug checking
};


Active_jumps* new_Active_jumps(void)
{
    Active_jumps* jumps = memory_alloc_item(Active_jumps);
    if (jumps == NULL)
        return NULL;

    jumps->jumps = NULL;
    jumps->use_count = 0;

    jumps->jumps = new_AAtree(
            (AAtree_item_cmp*)Jump_context_cmp, (AAtree_item_destroy*)del_Jump_context);

    if (jumps->jumps == NULL)
    {
        del_Active_jumps(jumps);
        return NULL;
    }

    return jumps;
}


void Active_jumps_add_context(Active_jumps* jumps, AAnode* handle)
{
    assert(jumps != NULL);
    assert(handle != NULL);

    AAtree_attach(jumps->jumps, handle);

    ++jumps->use_count;

    return;
}


Jump_context* Active_jumps_get_next_context(
        const Active_jumps* jumps,
        const Pat_inst_ref* piref,
        const Tstamp* row,
        int ch_num,
        int order)
{
    assert(jumps != NULL);
    assert(piref != NULL);
    assert(piref->pat >= 0);
    assert(piref->inst >= 0);
    assert(row != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(order >= 0);

    Jump_context* key = JUMP_CONTEXT_AUTO;
    key->piref = *piref;
    Tstamp_copy(&key->row, row);
    key->ch_num = ch_num;
    key->order = order;

    Jump_context* jc = AAtree_get_at_least(jumps->jumps, key);
    if (jc == NULL ||
            jc->piref.pat != key->piref.pat ||
            jc->piref.inst != key->piref.inst)
        return NULL;

    return jc;
}


AAnode* Active_jumps_remove_context(Active_jumps* jumps, const Jump_context* jc)
{
    assert(jumps != NULL);
    assert(jc != NULL);
    assert(AAtree_contains(jumps->jumps, jc));

    assert(jumps->use_count > 0);
    --jumps->use_count;

    return AAtree_detach(jumps->jumps, jc);
}


void Active_jumps_reset(Active_jumps* jumps, Jump_cache* jcache)
{
    assert(jumps != NULL);
    assert(jcache != NULL);

    Jump_context* key = JUMP_CONTEXT_AUTO;
    key->piref.pat = -1;
    key->piref.inst = -1;

    // Move jump contexts to cache (without iterator as the tree is modified)
    const Jump_context* cur = AAtree_get_at_least(jumps->jumps, key);
    while (cur != NULL)
    {
        AAnode* handle = AAtree_detach(jumps->jumps, cur);
        assert(handle != NULL);
        Jump_cache_release_context(jcache, handle);

        assert(jumps->use_count > 0);
        --jumps->use_count;

        cur = AAtree_get_at_least(jumps->jumps, key);
    }

    return;
}


void del_Active_jumps(Active_jumps* jumps)
{
    if (jumps == NULL)
        return;

    assert(jumps->use_count == 0);

    del_AAtree(jumps->jumps);
    memory_free(jumps);

    return;
}


