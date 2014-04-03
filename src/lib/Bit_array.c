

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdint.h>
#include <stdio.h>

#include <Bit_array.h>
#include <debug/assert.h>
#include <memory.h>


struct Bit_array
{
    size_t size;
    uint8_t* bits;
};


static size_t byte_index(size_t bit_index)
{
    return bit_index >> 3;
}


static size_t bit_offset(size_t bit_index)
{
    return bit_index & 0x7;
}


Bit_array* new_Bit_array(size_t size)
{
    assert(size > 0);

    Bit_array* ba = memory_alloc_item(Bit_array);
    if (ba == NULL)
        return NULL;
    ba->size = size;
    ba->bits = NULL;

    ba->bits = memory_calloc_items(uint8_t, byte_index(size) + 1);
    if (ba->bits == NULL)
    {
        del_Bit_array(ba);
        return NULL;
    }

    return ba;
}


void Bit_array_set(Bit_array* ba, size_t index, bool value)
{
    assert(ba != NULL);
    assert(index < ba->size);

    uint8_t mask = 1 << bit_offset(index);
    assert(mask != 0);

    if (value)
        ba->bits[byte_index(index)] |= mask;
    else
        ba->bits[byte_index(index)] &= ~mask;

    return;
}


bool Bit_array_get(const Bit_array* ba, size_t index)
{
    assert(ba != NULL);
    assert(index < ba->size);

    uint8_t mask = 1 << bit_offset(index);
    assert(mask != 0);

    return (ba->bits[byte_index(index)] & mask) != 0;
}


void del_Bit_array(Bit_array* ba)
{
    if (ba == NULL)
        return;

    memory_free(ba->bits);
    memory_free(ba);
    return;
}


