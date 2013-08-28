

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <AAtree.h>
#include <Hit_map.h>
#include <memory.h>
#include <xassert.h>


struct Hit_map
{
    AAtree* hits[KQT_HITS_MAX];
};


typedef struct Random_list
{
    double force;
    int entry_count;
    Sample_entry entries[HIT_MAP_RANDOMS_MAX];
} Random_list;


static int Random_list_cmp(
        const Random_list* list1,
        const Random_list* list2);


Hit_map* new_Hit_map_from_string(char* str, Read_state* state)
{
    assert(state != NULL);

    if (state->error)
        return NULL;

    Hit_map* map = memory_alloc_item(Hit_map);
    if (map == NULL)
        return NULL;

    for (int i = 0; i < KQT_HITS_MAX; ++i)
        map->hits[i] = NULL;

    if (str == NULL)
        return map;

    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Hit_map(map);
        return NULL;
    }

    str = read_const_char(str, ']', state);
    if (!state->error)
        return map;

    Read_state_clear_error(state);

    bool expect_list = true;
    while (expect_list)
    {
        Random_list* list = memory_alloc_item(Random_list);
        if (list == NULL)
        {
            del_Hit_map(map);
            return NULL;
        }
        str = read_const_char(str, '[', state);

        str = read_const_char(str, '[', state);
        int64_t hit_index = -1;
        str = read_int(str, &hit_index, state);
        str = read_const_char(str, ',', state);
        double force = NAN;
        str = read_double(str, &force, state);
        str = read_const_char(str, ']', state);
        str = read_const_char(str, ',', state);
        str = read_const_char(str, '[', state);
        if (state->error)
        {
            memory_free(list);
            del_Hit_map(map);
            return NULL;
        }

        if (hit_index < 0 || hit_index >= KQT_HITS_MAX)
        {
            Read_state_set_error(
                    state,
                    "Mapping hit index is outside [0, %d)",
                    KQT_HITS_MAX);
            memory_free(list);
            del_Hit_map(map);
            return NULL;
        }

        if (!isfinite(force))
        {
            Read_state_set_error(
                    state,
                    "Mapping force is not finite",
                    KQT_HITS_MAX);
            memory_free(list);
            del_Hit_map(map);
            return NULL;
        }

        if (map->hits[hit_index] == NULL)
        {
            map->hits[hit_index] = new_AAtree(
                    (int (*)(const void*, const void*))Random_list_cmp,
                    memory_free);
            if (map->hits[hit_index] == NULL)
            {
                memory_free(list);
                del_Hit_map(map);
                return NULL;
            }
        }

        list->force = force;
        list->entry_count = 0;
        if (!AAtree_ins(map->hits[hit_index], list))
        {
            memory_free(list);
            del_Hit_map(map);
            return NULL;
        }

        bool expect_entry = true;
        while (expect_entry && list->entry_count < HIT_MAP_RANDOMS_MAX)
        {
            str = Sample_entry_parse(
                    &list->entries[list->entry_count], str, state);
            if (state->error)
            {
                del_Hit_map(map);
                return NULL;
            }
            ++list->entry_count;
            check_next(str, state, expect_entry);
        }

        str = read_const_char(str, ']', state);

        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_Hit_map(map);
            return NULL;
        }

        check_next(str, state, expect_list);
    }

    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_Hit_map(map);
        return NULL;
    }

    return map;
}


const Sample_entry* Hit_map_get_entry(
        const Hit_map* map,
        int hit_index,
        double force,
        Random* random)
{
    assert(map != NULL);
    assert(hit_index >= 0);
    assert(hit_index < KQT_HITS_MAX);
    assert(isfinite(force) || force == -INFINITY);
    assert(random != NULL);

    AAtree* forces = map->hits[hit_index];
    if (forces == NULL)
    {
        fprintf(stderr, "no forces\n");
        return NULL;
    }

    Random_list* key = &(Random_list){ .force = force };
    Random_list* greater = AAtree_get_at_least(forces, key);
    Random_list* smaller = AAtree_get_at_most(forces, key);
    Random_list* list = NULL;

    if (greater == NULL)
        list = smaller;
    else if (smaller == NULL)
        list = greater;
    else if (fabs(greater->force - force) < fabs(smaller->force - force))
        list = greater;
    else
        list = smaller;

    if (list == NULL)
        return NULL;

    int index = Random_get_index(random, list->entry_count);
    return &list->entries[index];
}


void del_Hit_map(Hit_map* map)
{
    if (map == NULL)
        return;

    for (int i = 0; i < KQT_HITS_MAX; ++i)
        del_AAtree(map->hits[i]);

    memory_free(map);

    return;
}


static int Random_list_cmp(
        const Random_list* list1,
        const Random_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);

    if (list1->force < list2->force)
        return -1;
    else if (list1->force > list2->force)
        return 1;

    return 0;
}


