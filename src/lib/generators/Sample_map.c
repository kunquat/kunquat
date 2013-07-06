

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <math.h>

#include <memory.h>
#include <pitch_t.h>
#include <Sample_map.h>
#include <xassert.h>


struct Sample_map
{
    AAtree* map;
    AAiter* iter;
};


typedef struct Random_list
{
    pitch_t freq;
    double cents;
    double force;
    int entry_count;
    Sample_entry entries[SAMPLE_MAP_RANDOMS_MAX];
} Random_list;


static int Random_list_cmp(const Random_list* list1, const Random_list* list2);

static void del_Random_list(Random_list* list);

static double distance(Random_list* list, Random_list* key);


static int Random_list_cmp(const Random_list* list1, const Random_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);
    if (list1->cents < list2->cents)
    {
        return -1;
    }
    else if (list1->cents > list2->cents)
    {
        return 1;
    }
    if (list1->force < list2->force)
    {
        return -1;
    }
    if (list1->force > list2->force)
    {
        return 1;
    }
    return 0;
}


static void del_Random_list(Random_list* list)
{
    memory_free(list);
    return;
}


Sample_map* new_Sample_map_from_string(char* str, Read_state* state)
{
    if (state != NULL && state->error)
    {
        return NULL;
    }
    Sample_map* map = memory_alloc_item(Sample_map);
    if (map == NULL)
    {
        return NULL;
    }
    map->map = NULL;
    map->iter = NULL;
    map->map = new_AAtree((int (*)(const void*, const void*))Random_list_cmp,
                          (void (*)(void*))del_Random_list);
    if (map->map == NULL)
    {
        del_Sample_map(map);
        return NULL;
    }
    map->iter = new_AAiter(map->map);
    if (map->iter == NULL)
    {
        del_Sample_map(map);
        return NULL;
    }
    if (str == NULL)
    {
        return map;
    }
    assert(state != NULL);
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Sample_map(map);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return map;
    }
    Read_state_clear_error(state);
    bool expect_list = true;
    while (expect_list)
    {
        Random_list* list = memory_alloc_item(Random_list);
        if (list == NULL)
        {
            del_Sample_map(map);
            return NULL;
        }
        str = read_const_char(str, '[', state);

        str = read_const_char(str, '[', state);
        double cents = NAN;
        str = read_double(str, &cents, state);
        str = read_const_char(str, ',', state);
        double force = NAN;
        str = read_double(str, &force, state);
        str = read_const_char(str, ']', state);
        str = read_const_char(str, ',', state);
        str = read_const_char(str, '[', state);
        if (state->error)
        {
            memory_free(list);
            del_Sample_map(map);
            return NULL;
        }
        if (!isfinite(cents))
        {
            Read_state_set_error(state, "Mapping cents is not finite");
            memory_free(list);
            del_Sample_map(map);
            return NULL;
        }
        if (!isfinite(force))
        {
            Read_state_set_error(state, "Mapping force is not finite");
            memory_free(list);
            del_Sample_map(map);
            return NULL;
        }
        list->freq = exp2(cents / 1200) * 440;
        list->cents = cents;
        list->force = force;
        list->entry_count = 0;
        if (!AAtree_ins(map->map, list))
        {
            Read_state_set_error(state, "Couldn't allocate memory for sample mapping");
            memory_free(list);
            del_Sample_map(map);
            return NULL;
        }
        bool expect_entry = true;
        while (expect_entry && list->entry_count < SAMPLE_MAP_RANDOMS_MAX)
        {
            str = Sample_entry_parse(&list->entries[list->entry_count],
                                     str, state);
            if (state->error)
            {
                del_Sample_map(map);
                return NULL;
            }
            list->entries[list->entry_count].ref_freq = list->freq;
            ++list->entry_count;
            check_next(str, state, expect_entry);
        }
        str = read_const_char(str, ']', state);

        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_Sample_map(map);
            return NULL;
        }
        check_next(str, state, expect_list);
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_Sample_map(map);
        return NULL;
    }
    return map;
}


bool Sample_map_add_entry(Sample_map* map,
                          double cents,
                          double force,
                          Sample_entry* entry)
{
    assert(map != NULL);
    assert(isfinite(cents));
    assert(isfinite(force));
    assert(entry != NULL);
    Random_list* key = &(Random_list){ .force = force,
                                       .cents = cents };
    Random_list* list = AAtree_get_exact(map->map, key);
    if (list == NULL)
    {
        Random_list* list = memory_alloc_item(Random_list);
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
    if (list->entry_count >= SAMPLE_MAP_RANDOMS_MAX)
    {
        assert(list->entry_count == SAMPLE_MAP_RANDOMS_MAX);
        return false;
    }
    list->entries[list->entry_count].ref_freq = list->freq;
    list->entries[list->entry_count].cents = entry->cents;
    list->entries[list->entry_count].vol_scale = entry->vol_scale;
    list->entries[list->entry_count].sample = entry->sample;
    ++list->entry_count;
    return true;
}


static double distance(Random_list* list, Random_list* key)
{
    assert(list != NULL);
    assert(key != NULL);
    double tone_d = (list->cents - key->cents) * 64;
    double force_d = list->force - key->force;
    return hypot(tone_d, force_d);
}


const Sample_entry* Sample_map_get_entry(Sample_map* map,
                                         double cents,
                                         double force,
                                         Random* random)
{
    assert(map != NULL);
    assert(isfinite(cents));
    assert(isfinite(force) || (isinf(force) && force < 0));
    assert(random != NULL);
    Random_list* key = &(Random_list){ .force = force,
                                       .freq = NAN,
                                       .cents = cents };
    Random_list* estimate_low = AAiter_get_at_most(map->iter, key);
    Random_list* choice = NULL;
    double choice_d = INFINITY;
    if (estimate_low != NULL)
    {
        choice = estimate_low;
        choice_d = distance(choice, key);
        double min_tone = key->cents - choice_d;
        Random_list* candidate = AAiter_get_prev(map->iter);
        while (candidate != NULL && candidate->cents >= min_tone)
        {
            double d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                min_tone = key->cents - choice_d;
            }
            candidate = AAiter_get_prev(map->iter);
        }
    }
    Random_list* estimate_high = AAiter_get_at_least(map->iter, key);
    if (estimate_high != NULL)
    {
        double d = distance(estimate_high, key);
        if (choice == NULL || choice_d > d)
        {
            choice = estimate_high;
            choice_d = d;
        }
        double max_tone = key->cents + choice_d;
        Random_list* candidate = AAiter_get_next(map->iter);
        while (candidate != NULL && candidate->cents <= max_tone)
        {
            d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                max_tone = key->cents + choice_d;
            }
            candidate = AAiter_get_next(map->iter);
        }
    }
    if (choice == NULL)
    {
//        fprintf(stderr, "empty map\n");
        return NULL;
    }
    assert(choice->entry_count > 0);
    assert(choice->entry_count < SAMPLE_MAP_RANDOMS_MAX);
//    state->middle_tone = choice->freq;
    int index = Random_get_index(random, choice->entry_count);
    assert(index >= 0);
//    fprintf(stderr, "%d\n", index);
    return &choice->entries[index];
}


void del_Sample_map(Sample_map* map)
{
    if (map == NULL)
    {
        return;
    }
    del_AAiter(map->iter);
    del_AAtree(map->map);
    memory_free(map);
    return;
}


