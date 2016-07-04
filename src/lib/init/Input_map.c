

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


#include <init/Input_map.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>

#include <stdint.h>
#include <stdlib.h>


typedef struct Entry
{
    int32_t input;
    int32_t output;
} Entry;


#define ENTRY_KEY(in) (&(Entry){ .input = (in), .output = -1 })


static int Entry_cmp(const Entry* e1, const Entry* e2)
{
    rassert(e1 != NULL);
    rassert(e1->input >= 0);
    rassert(e2 != NULL);
    rassert(e2->input >= 0);

    return e1->input - e2->input;
}


struct Input_map
{
    AAtree* map;
    int32_t num_inputs;
    int32_t num_outputs;
};


static bool read_entry(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);
    ignore(index);

    Input_map* im = userdata;

    int64_t in = 0;
    int64_t out = 0;
    if (!Streader_readf(sr, "[%i, %i]", &in, &out))
        return false;

    if (in < 0 || in >= im->num_inputs)
    {
        Streader_set_error(
                sr,
                "Input ID %" PRId64 " out of range [0, %" PRId32 ")",
                in, im->num_inputs);
        return false;
    }

    if (out < 0 || out >= im->num_outputs)
    {
        Streader_set_error(
                sr,
                "Output ID %" PRId64 " out of range [0, %" PRId32 ")",
                out, im->num_outputs);
        return false;
    }

    const Entry* key = ENTRY_KEY((int32_t)in);
    if (AAtree_contains(im->map, key))
    {
        Streader_set_error(sr, "Duplicate entry for input %" PRId64, in);
        return false;
    }

    Entry* entry = memory_alloc_item(Entry);
    if (entry == NULL)
    {
        Error_set(
                &sr->error,
                ERROR_MEMORY,
                "Could not allocate memory for input map");
        return false;
    }

    entry->input = (int32_t)in;
    entry->output = (int32_t)out;

    if (!AAtree_ins(im->map, entry))
    {
        memory_free(entry);
        Error_set(
                &sr->error,
                ERROR_MEMORY,
                "Could not allocate memory for input map");
        return false;
    }

    return true;
}


Input_map* new_Input_map(Streader* sr, int32_t num_inputs, int32_t num_outputs)
{
    rassert(sr != NULL);
    rassert(num_inputs > 0);
    rassert(num_outputs > 0);

    Input_map* im = memory_alloc_item(Input_map);
    if (im == NULL)
        return NULL;

    im->map = NULL;
    im->num_inputs = num_inputs;
    im->num_outputs = num_outputs;

    im->map = new_AAtree(
            (AAtree_item_cmp*)Entry_cmp, (AAtree_item_destroy*)memory_free);
    if (im->map == NULL)
    {
        del_Input_map(im);
        return NULL;
    }

    if (!Streader_read_list(sr, read_entry, im))
    {
        del_Input_map(im);
        return NULL;
    }

    return im;
}


bool Input_map_is_valid(const Input_map* im, const Bit_array* existents)
{
    rassert(im != NULL);
    rassert(existents != NULL);

    const Entry* key = ENTRY_KEY(0);
    AAiter* iter = AAiter_init(AAITER_AUTO, im->map);

    const Entry* pair = AAiter_get_at_least(iter, key);
    while (pair != NULL)
    {
        if (!Bit_array_get(existents, pair->input))
            return false;

        pair = AAiter_get_next(iter);
    }

    return true;
}


int32_t Input_map_get_device_index(const Input_map* im, int32_t input_id)
{
    rassert(im != NULL);
    rassert(input_id >= 0);
    rassert(input_id < im->num_inputs);

    const Entry* key = ENTRY_KEY(input_id);
    const Entry* result = AAtree_get_exact(im->map, key);
    if (result == NULL)
        return -1;

    return result->output;
}


void del_Input_map(Input_map* im)
{
    if (im == NULL)
        return;

    del_AAtree(im->map);
    memory_free(im);

    return;
}


