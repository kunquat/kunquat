

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <inttypes.h>
#include <stdint.h>

#include <AAtree.h>
#include <Order_list.h>
#include <Pat_inst_ref.h>
#include <Vector.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct Index_mapping
{
    Pat_inst_ref p;
    size_t ol_index;
} Index_mapping;

#define INDEX_MAPPING_AUTO (&(Index_mapping){ .ol_index = 0 })

Index_mapping* new_Index_mapping(Index_mapping* im)
{
    assert(im != NULL);

    Index_mapping* new_im = xalloc(Index_mapping);
    if (new_im == NULL)
        return NULL;

    *new_im = *im;
    return new_im;
}


struct Order_list
{
    Vector* pat_insts;
    AAtree* index_map;
};


Order_list* new_Order_list(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
        return NULL;

    // Create the base structure
    Order_list* ol = xalloc(Order_list);
    if (ol == NULL)
        return NULL;
    ol->pat_insts = NULL;
    ol->index_map = NULL;

    // Create Pattern instance reference vector
    ol->pat_insts = new_Vector(sizeof(Pat_inst_ref));
    if (ol->pat_insts == NULL)
    {
        del_Order_list(ol);
        return NULL;
    }

    // Create reverse index of ol->pat_insts
    ol->index_map = new_AAtree(
            (int (*)(const void*, const void*))Pat_inst_ref_cmp,
            free);
    if (ol->index_map == NULL)
    {
        del_Order_list(ol);
        return NULL;
    }

    // List is empty by default
    if (str == NULL)
        return ol;

    // Read the list of Pattern instance references
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_Order_list(ol);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        // List not empty, process elements
        Read_state_clear_error(state);

        bool expect_pair = true;
        int32_t index = 0;
        while (expect_pair)
        {
            // Read the Pattern instance reference
            Pat_inst_ref* p = PAT_INST_REF_AUTO;
            str = read_pat_inst_ref(str, p, state);
            if (state->error)
            {
                del_Order_list(ol);
                return NULL;
            }

            // Check if the Pattern instance is already used
            Index_mapping* key = INDEX_MAPPING_AUTO;
            key->p = *p;
            key->ol_index = index;
            if (AAtree_contains(ol->index_map, key))
            {
                Read_state_set_error(state,
                        "Duplicate occurrence of pattern instance"
                        " [%" PRId16 ", %" PRId16 "]", p->pat, p->inst);
                del_Order_list(ol);
                return NULL;
            }

            // Add the reference to our containers
            if (!Vector_append(ol->pat_insts, p))
            {
                del_Order_list(ol);
                return NULL;
            }

            Index_mapping* im = new_Index_mapping(key);
            if (im == NULL || !AAtree_ins(ol->index_map, im))
            {
                xfree(im);
                del_Order_list(ol);
                return NULL;
            }

            ++index;
            check_next(str, state, expect_pair);
        }
        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_Order_list(ol);
            return NULL;
        }
    }

    return ol;
}


size_t Order_list_get_len(const Order_list* ol)
{
    assert(ol != NULL);

    return Vector_size(ol->pat_insts);
}


Pat_inst_ref* Order_list_get_pat_inst_ref(const Order_list* ol, size_t index)
{
    assert(ol != NULL);
    assert(index < Order_list_get_len(ol));

    return Vector_get_ref(ol->pat_insts, index);
}


void del_Order_list(Order_list* ol)
{
    if (ol == NULL)
        return;

    del_Vector(ol->pat_insts);
    del_AAtree(ol->index_map);
    xfree(ol);
    return;
}


