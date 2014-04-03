

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <debug/assert.h>
#include <math_common.h>
#include <serialise.h>
#include <Value.h>


Value* Value_copy(Value* restrict dest, const Value* restrict src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(dest != src);

    memcpy(dest, src, sizeof(Value));

    return dest;
}


bool Value_convert(Value* dest, const Value* src, Value_type new_type)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(new_type > VALUE_TYPE_NONE);
    assert(new_type < VALUE_TYPE_COUNT);

    switch (new_type)
    {
        case VALUE_TYPE_INT:
        {
            if (src->type == VALUE_TYPE_FLOAT)
            {
                dest->type = VALUE_TYPE_INT;
                dest->value.int_type = src->value.float_type;
            }
            else if (src->type != VALUE_TYPE_INT)
                return false;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            if (src->type == VALUE_TYPE_INT)
            {
                dest->type = VALUE_TYPE_FLOAT;
                dest->value.float_type = src->value.int_type;
            }
            else if (src->type != VALUE_TYPE_FLOAT)
                return false;
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            if (src->type == VALUE_TYPE_INT)
            {
                dest->type = VALUE_TYPE_TSTAMP;
                Tstamp_set(
                        &dest->value.Tstamp_type,
                        src->value.int_type, 0);
            }
            else if (src->type == VALUE_TYPE_FLOAT)
            {
                dest->type = VALUE_TYPE_TSTAMP;
                double beats_f = src->value.float_type;
                double beats = floor(beats_f);
                Tstamp_set(
                        &dest->value.Tstamp_type,
                        beats,
                        (beats_f - beats) * KQT_TSTAMP_BEAT);
            }
            else if (src->type != VALUE_TYPE_TSTAMP)
                return false;
        }
        break;

        default:
        {
            // Other types don't support conversions
            if (src->type != new_type)
                return false;
        }
        break;
    }

    return true;
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
        }
        break;

        case VALUE_TYPE_BOOL:
        {
            return serialise_bool(str, len, value->value.bool_type);
        }
        break;

        case VALUE_TYPE_INT:
        {
            return serialise_int(str, len, value->value.int_type);
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            return serialise_float(str, len, value->value.float_type);
        }
        break;

        case VALUE_TYPE_REAL:
        {
            return serialise_Real(str, len, &value->value.Real_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            return serialise_Tstamp(str, len, &value->value.Tstamp_type);
        }
        break;

        case VALUE_TYPE_STRING:
        {
            int print_len = snprintf(
                    str, len, "\"%s\"", value->value.string_type);
            return MIN(len - 1, print_len);
        }
        break;

        case VALUE_TYPE_PAT_INST_REF:
        {
            return serialise_Pat_inst_ref(
                    str, len, &value->value.Pat_inst_ref_type);
        }
        break;

        default:
            assert(false);
    }

    assert(false);
    return 0;
}


