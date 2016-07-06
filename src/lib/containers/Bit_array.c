

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


#include <containers/Bit_array.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>


struct Bit_array
{
    int64_t size;
    uint8_t* bits;
};


static int64_t byte_index(int64_t bit_index)
{
    return bit_index >> 3;
}


static int64_t bit_offset(int64_t bit_index)
{
    return bit_index & 0x7;
}


Bit_array* new_Bit_array(int64_t size)
{
    rassert(size > 0);

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


void Bit_array_clear(Bit_array* ba)
{
    rassert(ba != NULL);

    memset(ba->bits, 0, (size_t)byte_index(ba->size));

    return;
}


void Bit_array_set(Bit_array* ba, int64_t index, bool value)
{
    rassert(ba != NULL);
    rassert(index < ba->size);

    const uint8_t mask = (uint8_t)(1 << bit_offset(index));
    rassert(mask != 0);

    if (value)
        ba->bits[byte_index(index)] |= mask;
    else
        ba->bits[byte_index(index)] &= (uint8_t)~mask;

    return;
}


bool Bit_array_get(const Bit_array* ba, int64_t index)
{
    rassert(ba != NULL);
    rassert(index < ba->size);

    const uint8_t mask = (uint8_t)(1 << bit_offset(index));
    rassert(mask != 0);

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


