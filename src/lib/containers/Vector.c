

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


#include <containers/Vector.h>

#include <debug/assert.h>
#include <memory.h>

#include <string.h>


struct Vector
{
    int64_t elem_size;
    int64_t size;
    int64_t cap;
    char* elems;
};


#define VECTOR_INIT_CAP 4


static bool Vector_set_capacity(Vector* v, int64_t cap);


Vector* new_Vector(int64_t elem_size)
{
    assert(elem_size > 0);

    Vector* v = memory_alloc_item(Vector);
    if (v == NULL)
        return NULL;

    v->elem_size = elem_size;
    v->size = 0;
    v->cap = VECTOR_INIT_CAP;
    v->elems = memory_alloc_items(char, v->cap * v->elem_size);
    if (v->elems == NULL)
    {
        del_Vector(v);
        return NULL;
    }

    return v;
}


int64_t Vector_size(const Vector* v)
{
    assert(v != NULL);
    return v->size;
}


void Vector_get(const Vector* v, int64_t index, void* dest)
{
    assert(v != NULL);
    assert(index < v->size);
    assert(dest != NULL);

    memcpy(dest, v->elems + (v->elem_size * index), (size_t)v->elem_size);

    return;
}


void* Vector_get_ref(const Vector* v, int64_t index)
{
    assert(v != NULL);
    assert(index < v->size);

    return v->elems + (v->elem_size * index);
}


bool Vector_append(Vector* v, const void* elem)
{
    assert(v != NULL);
    assert(elem != NULL);

    if (v->size >= v->cap)
    {
        assert(v->size == v->cap);
        if (!Vector_set_capacity(v, v->cap * 2))
            return false;
    }

    memcpy(v->elems + (v->elem_size * v->size), elem, (size_t)v->elem_size);
    ++v->size;

    return true;
}


static bool Vector_set_capacity(Vector* v, int64_t cap)
{
    assert(v != NULL);
    assert(cap > 0);

    if (cap == v->cap)
        return true;

    char* new_elems = memory_realloc_items(char, cap * v->elem_size, v->elems);
    if (new_elems == NULL)
        return false;
    v->elems = new_elems;
    v->cap = cap;

    if (v->size > v->cap)
        v->size = v->cap;

    return true;
}


void del_Vector(Vector* v)
{
    if (v == NULL)
        return;

    memory_free(v->elems);
    memory_free(v);

    return;
}


