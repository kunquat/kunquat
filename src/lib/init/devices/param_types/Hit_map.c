

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Hit_map.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


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


static int Random_list_cmp(const Random_list* list1, const Random_list* list2)
{
    rassert(list1 != NULL);
    rassert(list2 != NULL);

    if (list1->force < list2->force)
        return -1;
    else if (list1->force > list2->force)
        return 1;

    return 0;
}


static bool read_random_list_entry(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);

    Random_list* list = userdata;

    if (index >= HIT_MAP_RANDOMS_MAX)
    {
        Streader_set_error(sr, "Too many hit map random list entries");
        return false;
    }

    if (!Sample_entry_parse(&list->entries[list->entry_count], sr))
        return false;

    ++list->entry_count;

    return true;
}

static bool read_mapping(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);
    rassert(userdata != NULL);

    Hit_map* map = userdata;

    Random_list* list = memory_alloc_item(Random_list);
    if (list == NULL)
    {
        del_Hit_map(map);
        Streader_set_memory_error(
                sr, "Could not allocate memory for hit map random list");
        return false;
    }

    int64_t hit_index = -1;
    double force = NAN;

    if (!Streader_readf(sr, "[[%i,%f],", &hit_index, &force))
    {
        memory_free(list);
        return false;
    }

    if (hit_index < 0 || hit_index >= KQT_HITS_MAX)
    {
        Streader_set_error(
                sr,
                "Mapping hit index is outside range [0, %d)",
                KQT_HITS_MAX);
        memory_free(list);
        return false;
    }

    if (!isfinite(force))
    {
        Streader_set_error(
                sr,
                "Mapping force is not finite",
                KQT_HITS_MAX);
        memory_free(list);
        return false;
    }

    if (map->hits[hit_index] == NULL)
    {
        map->hits[hit_index] = new_AAtree(
                (AAtree_item_cmp*)Random_list_cmp, (AAtree_item_destroy*)memory_free);
        if (map->hits[hit_index] == NULL)
        {
            Streader_set_memory_error(
                    sr, "Could not allocate memory for hit map");
            memory_free(list);
            return false;
        }
    }

    list->force = force;
    list->entry_count = 0;
    if (AAtree_contains(map->hits[hit_index], list))
    {
        Streader_set_error(
                sr,
                "Duplicate hit map entry with hit index %" PRId64 ", force %.2f",
                hit_index,
                force);
        memory_free(list);
        return false;
    }
    if (!AAtree_ins(map->hits[hit_index], list))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for hit map random list");
        memory_free(list);
        return false;
    }

    if (!Streader_read_list(sr, read_random_list_entry, list))
        return false;

    return Streader_match_char(sr, ']');
}

Hit_map* new_Hit_map_from_string(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Hit_map* map = memory_alloc_item(Hit_map);
    if (map == NULL)
    {
        Streader_set_memory_error(sr, "Could not allocate memory for hit map");
        return NULL;
    }

    for (int i = 0; i < KQT_HITS_MAX; ++i)
        map->hits[i] = NULL;

    if (!Streader_has_data(sr))
        return map;

    if (!Streader_read_list(sr, read_mapping, map))
    {
        del_Hit_map(map);
        return NULL;
    }

    return map;
}


const Sample_entry* Hit_map_get_entry(
        const Hit_map* map, int hit_index, double force, Random* random)
{
    rassert(map != NULL);
    rassert(hit_index >= 0);
    rassert(hit_index < KQT_HITS_MAX);
    rassert(isfinite(force) || force == -INFINITY);
    rassert(random != NULL);

    AAtree* forces = map->hits[hit_index];
    if (forces == NULL)
        return NULL;

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

    if (list->entry_count == 0)
        return NULL;

    const int index = Random_get_index(random, list->entry_count);
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


