

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


#include <Value.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <string/serialise.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool Value_type_is_realtime(Value_type type)
{
    assert(type >= VALUE_TYPE_NONE);
    assert(type < VALUE_TYPE_COUNT);

    return (type == VALUE_TYPE_BOOL) ||
        (type == VALUE_TYPE_INT) ||
        (type == VALUE_TYPE_FLOAT) ||
        (type == VALUE_TYPE_TSTAMP);
}


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

    if (src->type == new_type)
    {
        if (dest != src)
            Value_copy(dest, src);

        return true;
    }

    switch (new_type)
    {
        case VALUE_TYPE_INT:
        {
            if (src->type == VALUE_TYPE_FLOAT)
            {
                dest->type = VALUE_TYPE_INT;

                const double float_val = src->value.float_type;
                if ((float_val < INT64_MIN) || (float_val > INT64_MAX))
                    return false;

                dest->value.int_type = (int64_t)float_val;
            }
            else if (src->type == VALUE_TYPE_TSTAMP)
            {
                dest->type = VALUE_TYPE_INT;
                dest->value.int_type = Tstamp_get_beats(&src->value.Tstamp_type);
            }
            else
                return false;
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            if (src->type == VALUE_TYPE_INT)
            {
                dest->type = VALUE_TYPE_FLOAT;
                dest->value.float_type = (double)src->value.int_type;
            }
            else if (src->type == VALUE_TYPE_TSTAMP)
            {
                dest->type = VALUE_TYPE_FLOAT;

                const Tstamp* src_tstamp = &src->value.Tstamp_type;
                dest->value.float_type =
                    (double)Tstamp_get_beats(src_tstamp) +
                    (Tstamp_get_rem(src_tstamp) / (double)KQT_TSTAMP_BEAT);
            }
            else
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
                const double beats_f = src->value.float_type;
                const double beats = floor(beats_f);

                if ((beats < INT64_MIN) || (beats > INT64_MAX))
                    return false;

                Tstamp_set(
                        &dest->value.Tstamp_type,
                        (int64_t)beats,
                        (int32_t)((beats_f - beats) * KQT_TSTAMP_BEAT));
            }
            else
                return false;
        }
        break;

        default:
        {
            // Other types don't support conversions
            return false;
        }
        break;
    }

    return true;
}


int Value_serialise(const Value* value, int len, char* str)
{
    assert(value != NULL);
    assert(len > 0);
    assert(str != NULL);

    switch (value->type)
    {
        case VALUE_TYPE_NONE:
        {
            int print_len = snprintf(str, (size_t)len, "null");
            return min(len - 1, print_len);
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

        case VALUE_TYPE_TSTAMP:
        {
            return serialise_Tstamp(str, len, &value->value.Tstamp_type);
        }
        break;

        case VALUE_TYPE_STRING:
        {
            int print_len = snprintf(
                    str, (size_t)len, "\"%s\"", value->value.string_type);
            return min(len - 1, print_len);
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


