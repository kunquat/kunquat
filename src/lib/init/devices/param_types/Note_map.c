

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Note_map.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>

#include <math.h>
#include <stdlib.h>


struct Note_map
{
    AAtree* map;
};


typedef struct Random_list
{
    double freq;
    double cents;
    double force;
    int entry_count;
    Sample_entry entries[NOTE_MAP_RANDOMS_MAX];
} Random_list;


static int Random_list_cmp(const Random_list* list1, const Random_list* list2);

static void del_Random_list(Random_list* list);

static double distance(const Random_list* list, const Random_list* key);


static int Random_list_cmp(const Random_list* list1, const Random_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);

    if (list1->cents < list2->cents)
        return -1;
    else if (list1->cents > list2->cents)
        return 1;

    if (list1->force < list2->force)
        return -1;
    else if (list1->force > list2->force)
        return 1;

    return 0;
}


static void del_Random_list(Random_list* list)
{
    memory_free(list);
    return;
}


static bool read_random_list_entry(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    assert(userdata != NULL);

    if (index >= NOTE_MAP_RANDOMS_MAX)
    {
        Streader_set_error(sr, "Too many note map random list entries");
        return false;
    }

    Random_list* list = userdata;

    if (!Sample_entry_parse(&list->entries[list->entry_count], sr))
        return false;

    list->entries[list->entry_count].ref_freq = list->freq;
    ++list->entry_count;
    return true;
}

static bool read_mapping(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);
    assert(userdata != NULL);

    Note_map* map = userdata;

    Random_list* list = memory_alloc_item(Random_list);
    if (list == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for note map entry");
        return false;
    }

    double cents = NAN;
    double force = NAN;

    if (!Streader_readf(sr, "[[%f,%f],", &cents, &force))
    {
        memory_free(list);
        return false;
    }

    if (!isfinite(cents))
    {
        Streader_set_error(sr, "Mapping cents is not finite");
        memory_free(list);
        return false;
    }

    if (!isfinite(force))
    {
        Streader_set_error(sr, "Mapping force is not finite");
        memory_free(list);
        return false;
    }

    list->freq = exp2(cents / 1200) * 440;
    list->cents = cents;
    list->force = force;
    list->entry_count = 0;
    if (AAtree_contains(map->map, list))
    {
        Streader_set_error(
                sr,
                "Duplicate note map entry with pitch %.2f, force %.2f",
                list->cents,
                list->force);
        memory_free(list);
        return false;
    }
    if (!AAtree_ins(map->map, list))
    {
        Streader_set_memory_error(
                sr, "Couldn't allocate memory for note map entry");
        memory_free(list);
        return false;
    }

    if (!Streader_read_list(sr, read_random_list_entry, list))
        return false;

    return Streader_match_char(sr, ']');
}

Note_map* new_Note_map_from_string(Streader* sr)
{
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Note_map* map = memory_alloc_item(Note_map);
    if (map == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for note map");
        return NULL;
    }

    map->map = new_AAtree(
            (AAtree_item_cmp*)Random_list_cmp, (AAtree_item_destroy*)del_Random_list);
    if (map->map == NULL)
    {
        del_Note_map(map);
        Streader_set_memory_error(
                sr, "Could not allocate memory for note map");
        return NULL;
    }

    if (!Streader_has_data(sr))
        return map;

    if (!Streader_read_list(sr, read_mapping, map))
    {
        del_Note_map(map);
        return NULL;
    }

    return map;
}


bool Note_map_add_entry(Note_map* map, double cents, double force, Sample_entry* entry)
{
    assert(map != NULL);
    assert(isfinite(cents));
    assert(isfinite(force));
    assert(entry != NULL);

    Random_list* key = &(Random_list){ .force = force, .cents = cents };
    Random_list* list = AAtree_get_exact(map->map, key);
    if (list == NULL)
    {
        list = memory_alloc_item(Random_list);
        if (list == NULL || !AAtree_ins(map->map, list))
        {
            memory_free(list);
            return false;
        }
        list->freq = exp2(cents / 1200) * 440;
        list->cents = cents;
        list->force = force;
        list->entry_count = 0;
    }

    if (list->entry_count >= NOTE_MAP_RANDOMS_MAX)
    {
        assert(list->entry_count == NOTE_MAP_RANDOMS_MAX);
        return false;
    }

    list->entries[list->entry_count].ref_freq = list->freq;
    list->entries[list->entry_count].sample = entry->sample;
    list->entries[list->entry_count].cents = entry->cents;
    list->entries[list->entry_count].vol_scale = entry->vol_scale;
    ++list->entry_count;

    return true;
}


static double distance(const Random_list* list, const Random_list* key)
{
    assert(list != NULL);
    assert(key != NULL);
    const double tone_d = (list->cents - key->cents) * 64;
    const double force_d = list->force - key->force;
    return hypot(tone_d, force_d);
}


const Sample_entry* Note_map_get_entry(
        const Note_map* map, double cents, double force, Random* random)
{
    assert(map != NULL);
    assert(isfinite(cents));
    assert(isfinite(force) || (isinf(force) && force < 0));
    assert(random != NULL);

    const Random_list* key =
        &(Random_list){ .force = force, .freq = NAN, .cents = cents };
    AAiter* iter = AAiter_init(AAITER_AUTO, map->map);
    Random_list* estimate_low = AAiter_get_at_most(iter, key);
    Random_list* choice = NULL;
    double choice_d = INFINITY;

    if (estimate_low != NULL)
    {
        choice = estimate_low;
        choice_d = distance(choice, key);
        double min_tone = key->cents - choice_d;
        Random_list* candidate = AAiter_get_prev(iter);
        while (candidate != NULL && candidate->cents >= min_tone)
        {
            double d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                min_tone = key->cents - choice_d;
            }
            candidate = AAiter_get_prev(iter);
        }
    }

    Random_list* estimate_high = AAiter_get_at_least(iter, key);
    if (estimate_high != NULL)
    {
        double d = distance(estimate_high, key);
        if (choice == NULL || choice_d > d)
        {
            choice = estimate_high;
            choice_d = d;
        }

        double max_tone = key->cents + choice_d;
        Random_list* candidate = AAiter_get_next(iter);
        while (candidate != NULL && candidate->cents <= max_tone)
        {
            d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                max_tone = key->cents + choice_d;
            }
            candidate = AAiter_get_next(iter);
        }
    }

    if (choice == NULL)
    {
//        fprintf(stderr, "empty map\n");
        return NULL;
    }

    if (choice->entry_count == 0)
        return NULL;

    assert(choice->entry_count <= NOTE_MAP_RANDOMS_MAX);
//    state->middle_tone = choice->freq;
    const int index = Random_get_index(random, choice->entry_count);
    assert(index >= 0);
//    fprintf(stderr, "%d\n", index);

    return &choice->entries[index];
}


void del_Note_map(Note_map* map)
{
    if (map == NULL)
        return;

    del_AAtree(map->map);
    memory_free(map);

    return;
}


