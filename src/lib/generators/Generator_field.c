

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <generators/File_wavpack.h>
#include <Generator_field.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef union
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Real Real_type;
    Reltime Reltime_type;
    Sample* Sample_type;
    Sample_params Sample_params_type;
    Sample_map* Sample_map_type;
} Gen_fields;


struct Generator_field
{
    char key[100];
    Generator_field_type type;
    bool empty;
    Gen_fields data;
};


Generator_field* new_Generator_field(const char* key, void* data)
{
    assert(key != NULL);
    Generator_field_type type = GENERATOR_FIELD_NONE;
    size_t data_size = 0;
    if (string_has_suffix(key, ".jsonb"))
    {
        type = GENERATOR_FIELD_BOOL;
        data_size = sizeof(bool);
    }
    else if (string_has_suffix(key, ".jsoni"))
    {
        type = GENERATOR_FIELD_INT;
        data_size = sizeof(int64_t);
    }
    else if (string_has_suffix(key, ".jsonf"))
    {
        type = GENERATOR_FIELD_FLOAT;
        data_size = sizeof(double);
    }
    else if (string_has_suffix(key, ".jsonr"))
    {
        type = GENERATOR_FIELD_REAL;
        data_size = sizeof(Real);
    }
    else if (string_has_suffix(key, ".jsont"))
    {
        type = GENERATOR_FIELD_RELTIME;
        data_size = sizeof(Reltime);
    }
    else if (string_has_suffix(key, ".wv"))
    {
        type = GENERATOR_FIELD_WAVPACK;
        data_size = sizeof(Sample*);
    }
    else if (string_has_suffix(key, ".jsonsh"))
    {
        type = GENERATOR_FIELD_SAMPLE_PARAMS;
        data_size = sizeof(Sample_params);
    }
    else if (string_has_suffix(key, ".jsonsm"))
    {
        type = GENERATOR_FIELD_SAMPLE_MAP;
        data_size = sizeof(Sample_map*);
    }
    else
    {
        assert(false);
    }
    Generator_field* field = xalloc(Generator_field);
    if (field == NULL)
    {
        return NULL;
    }
    strncpy(field->key, key, 99);
    field->key[99] = '\0';
    field->type = type;
    if (data != NULL)
    {
        field->empty = false;
        memcpy(&field->data, data, data_size);
    }
    else
    {
        field->empty = true;
        memset(&field->data, 0, sizeof(Gen_fields));
    }
    return field;
}


Generator_field* new_Generator_field_from_data(const char* key,
                                               void* data,
                                               long length,
                                               Read_state* state)
{
    assert(key != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Generator_field* field = new_Generator_field(key, NULL);
    if (field == NULL)
    {
        return NULL;
    }
    if (!Generator_field_change(field, data, length, state))
    {
        del_Generator_field(field);
        return NULL;
    }
    return field;
}


bool Generator_field_change(Generator_field* field,
                            void* data,
                            long length,
                            Read_state* state)
{
    assert(field != NULL);
    assert(field->type != GENERATOR_FIELD_NONE);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    switch (field->type)
    {
        case GENERATOR_FIELD_BOOL:
        {
            if (data != NULL)
            {
                char* str = data;
                read_bool(str, &field->data.bool_type, state);
            }
        } break;
        case GENERATOR_FIELD_INT:
        {
            if (data != NULL)
            {
                char* str = data;
                read_int(str, &field->data.int_type, state);
            }
        } break;
        case GENERATOR_FIELD_FLOAT:
        {
            if (data != NULL)
            {
                char* str = data;
                read_double(str, &field->data.float_type, state);
            }
        } break;
        case GENERATOR_FIELD_REAL:
        {
            if (data != NULL)
            {
                assert(false); // TODO: implement
            }
        } break;
        case GENERATOR_FIELD_RELTIME:
        {
            if (data != NULL)
            {
                char* str = data;
                read_reltime(str, &field->data.Reltime_type, state);
            }
        } break;
        case GENERATOR_FIELD_WAVPACK:
        {
            Sample* sample = NULL;
            if (data != NULL)
            {
                sample = new_Sample();
                if (sample == NULL)
                {
                    return false;
                }
                if (!Sample_parse_wavpack(sample, data, length, state))
                {
                    del_Sample(sample);
                    return false;
                }
            }
            assert(!state->error);
            if (field->data.Sample_type != NULL)
            {
                del_Sample(field->data.Sample_type);
            }
            field->data.Sample_type = sample;
        } break;
        case GENERATOR_FIELD_SAMPLE_PARAMS:
        {
            if (!Sample_params_parse(&field->data.Sample_params_type,
                                     data, state))
            {
                return false;
            }
        } break;
        case GENERATOR_FIELD_SAMPLE_MAP:
        {
            Sample_map* map = new_Sample_map_from_string(data, state);
            if (map == NULL)
            {
                return false;
            }
            field->data.Sample_map_type = map;
        } break;
        default:
            assert(false);
    }
    if (state->error)
    {
        return false;
    }
    field->empty = data == NULL;
    return true;
}


int Generator_field_cmp(const Generator_field* field1,
                        const Generator_field* field2)
{
    assert(field1 != NULL);
    assert(field1->key != NULL);
    assert(field2 != NULL);
    assert(field2->key != NULL);
    return strcmp(field1->key, field2->key);
}


void Generator_field_set_empty(Generator_field* field, bool empty)
{
    assert(field != NULL);
    if (field->type != GENERATOR_FIELD_WAVPACK)
    {
        field->empty = empty;
    }
    return;
}


bool Generator_field_get_empty(Generator_field* field)
{
    assert(field != NULL);
    return field->empty;
}


bool Generator_field_modify(Generator_field* field, char* str)
{
    assert(field != NULL);
    assert(field->type != GENERATOR_FIELD_WAVPACK);
    assert(field->type != GENERATOR_FIELD_SAMPLE_MAP);
    Read_state* state = READ_STATE_AUTO;
    switch (field->type)
    {
        case GENERATOR_FIELD_BOOL:
        {
            read_bool(str, &field->data.bool_type, state);
        } break;
        case GENERATOR_FIELD_INT:
        {
            read_int(str, &field->data.int_type, state);
        } break;
        case GENERATOR_FIELD_FLOAT:
        {
            read_double(str, &field->data.float_type, state);
        } break;
        case GENERATOR_FIELD_RELTIME:
        {
            read_reltime(str, &field->data.Reltime_type, state);
        } break;
        default:
            assert(false);
    }
    if (state->error)
    {
        return false;
    }
    Generator_field_set_empty(field, false);
    return true;
}


bool* Generator_field_get_bool(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_BOOL);
    return &field->data.bool_type;
}


int64_t* Generator_field_get_int(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_INT);
    return &field->data.int_type;
}


double* Generator_field_get_float(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_FLOAT);
    return &field->data.float_type;
}


Real* Generator_field_get_real(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_REAL);
    return &field->data.Real_type;
}


Reltime* Generator_field_get_reltime(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_RELTIME);
    return &field->data.Reltime_type;
}


Sample* Generator_field_get_sample(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_WAVPACK);
    return field->data.Sample_type;
}


Sample_params* Generator_field_get_sample_params(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_SAMPLE_PARAMS);
    return &field->data.Sample_params_type;
}


Sample_map* Generator_field_get_sample_map(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_SAMPLE_MAP);
    return field->data.Sample_map_type;
}


void del_Generator_field(Generator_field* field)
{
    assert(field != NULL);
    if (field->type == GENERATOR_FIELD_WAVPACK &&
            field->data.Sample_type != NULL)
    {
        del_Sample(field->data.Sample_type);
    }
    if (field->type == GENERATOR_FIELD_SAMPLE_MAP &&
            field->data.Sample_map_type != NULL)
    {
        del_Sample_map(field->data.Sample_map_type);
    }
    xfree(field);
    return;
}


