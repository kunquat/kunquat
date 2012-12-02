

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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <math_common.h>
#include <serialise.h>
#include <Value.h>
#include <xassert.h>


Value* Value_copy(Value* restrict dest, const Value* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);
    memcpy(dest, src, sizeof(Value));
    return dest;
}


int Value_serialise(Value* value, int len, char* str)
{
    assert(value != NULL);
    assert(len > 0);
    assert(str != NULL);
    switch (value->type)
    {
        case VALUE_TYPE_NONE:
        {
            int print_len = snprintf(str, len, "null");
            return MIN(len - 1, print_len);
        } break;
        case VALUE_TYPE_BOOL:
        {
            return serialise_bool(str, len, value->value.bool_type);
        } break;
        case VALUE_TYPE_INT:
        {
            return serialise_int(str, len, value->value.int_type);
        } break;
        case VALUE_TYPE_FLOAT:
        {
            return serialise_float(str, len, value->value.float_type);
        } break;
        case VALUE_TYPE_REAL:
        {
            return serialise_Real(str, len, &value->value.Real_type);
        } break;
        case VALUE_TYPE_TIMESTAMP:
        {
            return serialise_Timestamp(str, len,
                    &value->value.Timestamp_type);
        } break;
        case VALUE_TYPE_STRING:
        {
            int print_len = snprintf(str, len, "\"%s\"",
                                     value->value.string_type);
            return MIN(len - 1, print_len);
        } break;
        case VALUE_TYPE_PAT_INSTANCE:
        {
            return serialise_Pat_instance(str, len,
                    &value->value.Pat_instance_type);
        } break;
        default:
            assert(false);
    }
    assert(false);
    return 0;
}


