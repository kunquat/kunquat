

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


#include <containers/Array.h>

#include <debug/assert.h>
#include <memory.h>

#include <string.h>


struct Array
{
    int64_t elem_size;
    int64_t size;
    int64_t cap;
    char* elems;
};


#define ARRAY_INIT_CAP 4


static bool Array_set_capacity(Array* a, int64_t cap);


Array* new_Array(int64_t elem_size)
{
    rassert(elem_size > 0);

    Array* a = memory_alloc_item(Array);
    if (a == NULL)
        return NULL;

    a->elem_size = elem_size;
    a->size = 0;
    a->cap = ARRAY_INIT_CAP;
    a->elems = memory_alloc_items(char, a->cap * a->elem_size);
    if (a->elems == NULL)
    {
        del_Array(a);
        return NULL;
    }

    return a;
}


int64_t Array_get_size(const Array* a)
{
    rassert(a != NULL);
    return a->size;
}


void Array_get_copy(const Array* a, int64_t index, void* dest)
{
    rassert(a != NULL);
    rassert(index < a->size);
    rassert(dest != NULL);

    memcpy(dest, a->elems + (a->elem_size * index), (size_t)a->elem_size);

    return;
}


void* Array_get_ref(const Array* a, int64_t index)
{
    rassert(a != NULL);
    rassert(index < a->size);

    return a->elems + (a->elem_size * index);
}


bool Array_append(Array* a, const void* elem)
{
    rassert(a != NULL);
    rassert(elem != NULL);

    if (a->size >= a->cap)
    {
        rassert(a->size == a->cap);
        if (!Array_set_capacity(a, a->cap * 2))
            return false;
    }

    memcpy(a->elems + (a->elem_size * a->size), elem, (size_t)a->elem_size);
    ++a->size;

    return true;
}


void Array_remove_at(Array* a, int64_t index)
{
    rassert(a != NULL);
    rassert(index >= 0);
    rassert(index < a->size);

    if (index + 1 < a->size)
    {
        char* dest = a->elems + (a->elem_size * index);
        const char* src = dest + a->elem_size;
        memmove(dest, src, (size_t)(a->elem_size * (a->size - (index + 1))));
    }

    --a->size;
    return;
}


void Array_clear(Array* a)
{
    rassert(a != NULL);

    Array_set_capacity(a, ARRAY_INIT_CAP);
    a->size = 0;

    return;
}


static bool Array_set_capacity(Array* a, int64_t cap)
{
    rassert(a != NULL);
    rassert(cap > 0);

    if (cap == a->cap)
        return true;

    char* new_elems = memory_realloc_items(char, cap * a->elem_size, a->elems);
    if (new_elems == NULL)
        return false;
    a->elems = new_elems;
    a->cap = cap;

    if (a->size > a->cap)
        a->size = a->cap;

    return true;
}


void del_Array(Array* a)
{
    if (a == NULL)
        return;

    memory_free(a->elems);
    memory_free(a);

    return;
}


