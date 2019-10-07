

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/sheet/Order_list.h>

#include <containers/AAtree.h>
#include <containers/Array.h>
#include <debug/assert.h>
#include <memory.h>
#include <Pat_inst_ref.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct Index_mapping
{
    Pat_inst_ref p;
    int ol_index;
} Index_mapping;

#define INDEX_MAPPING_AUTO (&(Index_mapping){ .ol_index = 0 })

static Index_mapping* new_Index_mapping(Index_mapping* im)
{
    rassert(im != NULL);

    Index_mapping* new_im = memory_alloc_item(Index_mapping);
    if (new_im == NULL)
        return NULL;

    *new_im = *im;
    return new_im;
}


struct Order_list
{
    Array* pat_insts;
    AAtree* index_map;
};


static bool read_piref(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);

    Order_list* ol = userdata;

    // Read the Pattern instance reference
    Pat_inst_ref* p = PAT_INST_REF_AUTO;
    if (!Streader_read_piref(sr, p))
        return false;

    // Check if the Pattern instance is already used
    Index_mapping* key = INDEX_MAPPING_AUTO;
    key->p = *p;
    key->ol_index = index;
    if (AAtree_contains(ol->index_map, key))
    {
        Streader_set_error(
                sr,
                "Duplicate occurrence of pattern instance"
                    " [%" PRId16 ", %" PRId16 "]",
                p->pat, p->inst);
        return false;
    }

    // Add the reference to our containers
    if (!Array_append(ol->pat_insts, p))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for order list");
        return false;
    }

    Index_mapping* im = new_Index_mapping(key);
    if (im == NULL || !AAtree_ins(ol->index_map, im))
    {
        memory_free(im);
        Streader_set_memory_error(
                sr, "Could not allocate memory for order list");
        return false;
    }

    return true;
}

Order_list* new_Order_list(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    // Create the base structure
    Order_list* ol = memory_alloc_item(Order_list);
    if (ol == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for order list");
        return NULL;
    }

    ol->pat_insts = NULL;
    ol->index_map = NULL;

    // Create Pattern instance reference array
    ol->pat_insts = new_Array(sizeof(Pat_inst_ref));
    if (ol->pat_insts == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for order list");
        del_Order_list(ol);
        return NULL;
    }

    // Create reverse index of ol->pat_insts
    ol->index_map = new_AAtree(
            (AAtree_item_cmp*)Pat_inst_ref_cmp, (AAtree_item_destroy*)memory_free);
    if (ol->index_map == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for order list");
        del_Order_list(ol);
        return NULL;
    }

    // List is empty by default
    if (!Streader_has_data(sr))
        return ol;

    // Read the list of Pattern instance references
    if (!Streader_read_list(sr, read_piref, ol))
    {
        del_Order_list(ol);
        return NULL;
    }

    return ol;
}


int Order_list_get_len(const Order_list* ol)
{
    rassert(ol != NULL);

    return (int)Array_get_size(ol->pat_insts);
}


Pat_inst_ref* Order_list_get_pat_inst_ref(const Order_list* ol, int index)
{
    rassert(ol != NULL);
    rassert(index < Order_list_get_len(ol));

    return Array_get_ref(ol->pat_insts, index);
}


bool Order_list_contains_pat_inst_ref(const Order_list* ol, const Pat_inst_ref* piref)
{
    rassert(ol != NULL);
    rassert(piref != NULL);

    for (int64_t i = 0; i < Array_get_size(ol->pat_insts); ++i)
    {
        const Pat_inst_ref* cur_piref = Array_get_ref(ol->pat_insts, i);
        if (Pat_inst_ref_cmp(cur_piref, piref) == 0)
            return true;
    }

    return false;
}


void del_Order_list(Order_list* ol)
{
    if (ol == NULL)
        return;

    del_Array(ol->pat_insts);
    del_AAtree(ol->index_map);
    memory_free(ol);
    return;
}


