

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
#include <assert.h>
#include <string.h>

#include <generators/File_wavpack.h>
#include <Generator_field.h>
#include <string_common.h>

#include <xmemory.h>


typedef union
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Real Real_type;
    Reltime Reltime_type;
    Sample* Sample_type;
} Gen_fields;


struct Generator_field
{
    Generator_field_type type;
    char* key;
    Gen_fields data;
};


Generator_field* new_Generator_field(const char* key, void* data)
{
    assert(key != NULL);
    assert(data != NULL);
    Generator_field_type type = GENERATOR_FIELD_NONE;
    size_t data_size = 0;
    if (string_has_suffix(key, ".b"))
    {
        type = GENERATOR_FIELD_BOOL;
        data_size = sizeof(bool);
    }
    else if (string_has_suffix(key, ".i"))
    {
        type = GENERATOR_FIELD_INT;
        data_size = sizeof(int64_t);
    }
    else if (string_has_suffix(key, ".f"))
    {
        type = GENERATOR_FIELD_FLOAT;
        data_size = sizeof(double);
    }
    else if (string_has_suffix(key, ".r"))
    {
        type = GENERATOR_FIELD_REAL;
        data_size = sizeof(Real);
    }
    else if (string_has_suffix(key, ".rt"))
    {
        type = GENERATOR_FIELD_RELTIME;
        data_size = sizeof(Reltime);
    }
    else if (string_has_suffix(key, ".wv"))
    {
        type = GENERATOR_FIELD_WAVPACK;
        data_size = sizeof(Sample*);
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
    field->key = xcalloc(char, strlen(key) + 1);
    if (field->key == NULL)
    {
        xfree(field);
        return NULL;
    }
    strcpy(field->key, key);
    field->type = type;
    memcpy(&field->data, data, data_size);
    return field;
}


Generator_field* new_Generator_field_from_string(const char* key,
                                                 void* data,
                                                 long length,
                                                 Read_state* state)
{
    assert(key != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    Gen_fields field_data;
    if (string_has_suffix(key, ".b"))
    {
        field_data.bool_type = false;
        if (data != NULL)
        {
            char* str = data;
            read_bool(str, &field_data.bool_type, state);
        }
    }
    else if (string_has_suffix(key, ".i"))
    {
        field_data.int_type = 0;
        if (data != NULL)
        {
            char* str = data;
            read_int(str, &field_data.int_type, state);
        }
    }
    else if (string_has_suffix(key, ".f"))
    {
        field_data.float_type = 0;
        if (data != NULL)
        {
            char* str = data;
            read_double(str, &field_data.float_type, state);
        }
    }
    else if (string_has_suffix(key, ".r"))
    {
        Real_init(&field_data.Real_type);
        if (data != NULL)
        {
            //char* str = data;
            assert(false); // TODO: implement
        }
    }
    else if (string_has_suffix(key, ".rt"))
    {
        Reltime_init(&field_data.Reltime_type);
        if (data != NULL)
        {
            char* str = data;
            read_reltime(str, &field_data.Reltime_type, state);
        }
    }
    else if (string_has_suffix(key, ".wv"))
    {
        field_data.Sample_type = NULL;
        if (data != NULL)
        {
            field_data.Sample_type = new_Sample();
            if (field_data.Sample_type == NULL)
            {
                return NULL;
            }
            if (!Sample_parse_wavpack(field_data.Sample_type,
                                      data, length, state))
            {
                del_Sample(field_data.Sample_type);
                return NULL;
            }
        }
    }
    else
    {
        assert(false);
    }
    if (state->error)
    {
        return NULL;
    }
    return new_Generator_field(key, &field_data);
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


bool Generator_field_get_bool(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_BOOL);
    return field->data.bool_type;
}


int64_t Generator_field_get_int(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_INT);
    return field->data.int_type;
}


double Generator_field_get_float(Generator_field* field)
{
    assert(field != NULL);
    assert(field->type == GENERATOR_FIELD_FLOAT);
    return field->data.float_type;
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


void del_Generator_field(Generator_field* field)
{
    assert(field != NULL);
    if (field->type == GENERATOR_FIELD_WAVPACK &&
            field->data.Sample_type != NULL)
    {
        del_Sample(field->data.Sample_type);
    }
    xfree(field->key);
    xfree(field);
    return;
}


