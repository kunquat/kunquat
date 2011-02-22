

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
#include <Device_field.h>
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
    Envelope* Envelope_type;
    Sample* Sample_type;
    Sample_params Sample_params_type;
    Sample_map* Sample_map_type;
} Dev_fields;


struct Device_field
{
    char key[100];
    Device_field_type type;
    bool empty;
    Dev_fields data;
};


Device_field* new_Device_field(const char* key, void* data)
{
    assert(key != NULL);
    Device_field_type type = DEVICE_FIELD_NONE;
    size_t data_size = 0;
    if (string_has_suffix(key, ".jsonb"))
    {
        type = DEVICE_FIELD_BOOL;
        data_size = sizeof(bool);
    }
    else if (string_has_suffix(key, ".jsoni"))
    {
        type = DEVICE_FIELD_INT;
        data_size = sizeof(int64_t);
    }
    else if (string_has_suffix(key, ".jsonf"))
    {
        type = DEVICE_FIELD_FLOAT;
        data_size = sizeof(double);
    }
    else if (string_has_suffix(key, ".jsonr"))
    {
        type = DEVICE_FIELD_REAL;
        data_size = sizeof(Real);
    }
    else if (string_has_suffix(key, ".jsont"))
    {
        type = DEVICE_FIELD_RELTIME;
        data_size = sizeof(Reltime);
    }
    else if (string_has_suffix(key, ".jsone"))
    {
        type = DEVICE_FIELD_ENVELOPE;
        data_size = sizeof(Envelope*);
    }
    else if (string_has_suffix(key, ".wv"))
    {
        type = DEVICE_FIELD_WAVPACK;
        data_size = sizeof(Sample*);
    }
    else if (string_has_suffix(key, ".jsonsh"))
    {
        type = DEVICE_FIELD_SAMPLE_PARAMS;
        data_size = sizeof(Sample_params);
    }
    else if (string_has_suffix(key, ".jsonsm"))
    {
        type = DEVICE_FIELD_SAMPLE_MAP;
        data_size = sizeof(Sample_map*);
    }
    else
    {
        assert(false);
    }
    Device_field* field = xalloc(Device_field);
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
        memset(&field->data, 0, sizeof(Dev_fields));
    }
    return field;
}


Device_field* new_Device_field_from_data(const char* key,
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
    Device_field* field = new_Device_field(key, NULL);
    if (field == NULL)
    {
        return NULL;
    }
    if (!Device_field_change(field, data, length, state))
    {
        del_Device_field(field);
        return NULL;
    }
    return field;
}


bool Device_field_change(Device_field* field,
                         void* data,
                         long length,
                         Read_state* state)
{
    assert(field != NULL);
    assert(field->type != DEVICE_FIELD_NONE);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    switch (field->type)
    {
        case DEVICE_FIELD_BOOL:
        {
            if (data != NULL)
            {
                char* str = data;
                read_bool(str, &field->data.bool_type, state);
            }
        } break;
        case DEVICE_FIELD_INT:
        {
            if (data != NULL)
            {
                char* str = data;
                read_int(str, &field->data.int_type, state);
            }
        } break;
        case DEVICE_FIELD_FLOAT:
        {
            if (data != NULL)
            {
                char* str = data;
                read_double(str, &field->data.float_type, state);
            }
        } break;
        case DEVICE_FIELD_REAL:
        {
            if (data != NULL)
            {
                assert(false); // TODO: implement
            }
        } break;
        case DEVICE_FIELD_RELTIME:
        {
            if (data != NULL)
            {
                char* str = data;
                read_reltime(str, &field->data.Reltime_type, state);
            }
        } break;
        case DEVICE_FIELD_ENVELOPE:
        {
            Envelope* env = NULL;
            if (data != NULL)
            {
                env = new_Envelope(32, -INFINITY, INFINITY, 0,
                                       -INFINITY, INFINITY, 0);
                if (env == NULL)
                {
                    return false;
                }
                char* str = data;
                Envelope_read(env, str, state);
                if (state->error)
                {
                    del_Envelope(env);
                    return false;
                }
            }
            assert(!state->error);
            if (field->data.Envelope_type != NULL)
            {
                del_Envelope(field->data.Envelope_type);
            }
            field->data.Envelope_type = env;
        } break;
        case DEVICE_FIELD_WAVPACK:
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
        case DEVICE_FIELD_SAMPLE_PARAMS:
        {
            if (!Sample_params_parse(&field->data.Sample_params_type,
                                     data, state))
            {
                return false;
            }
        } break;
        case DEVICE_FIELD_SAMPLE_MAP:
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


int Device_field_cmp(const Device_field* field1,
                     const Device_field* field2)
{
    assert(field1 != NULL);
    assert(field1->key != NULL);
    assert(field2 != NULL);
    assert(field2->key != NULL);
    return strcmp(field1->key, field2->key);
}


void Device_field_set_empty(Device_field* field, bool empty)
{
    assert(field != NULL);
    if (field->type != DEVICE_FIELD_WAVPACK)
    {
        field->empty = empty;
    }
    return;
}


bool Device_field_get_empty(Device_field* field)
{
    assert(field != NULL);
    return field->empty;
}


bool Device_field_modify(Device_field* field, char* str)
{
    assert(field != NULL);
    assert(field->type != DEVICE_FIELD_ENVELOPE);
    assert(field->type != DEVICE_FIELD_WAVPACK);
    assert(field->type != DEVICE_FIELD_SAMPLE_MAP);
    Read_state* state = READ_STATE_AUTO;
    switch (field->type)
    {
        case DEVICE_FIELD_BOOL:
        {
            read_bool(str, &field->data.bool_type, state);
        } break;
        case DEVICE_FIELD_INT:
        {
            read_int(str, &field->data.int_type, state);
        } break;
        case DEVICE_FIELD_FLOAT:
        {
            read_double(str, &field->data.float_type, state);
        } break;
        case DEVICE_FIELD_RELTIME:
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
    Device_field_set_empty(field, false);
    return true;
}


bool* Device_field_get_bool(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_BOOL);
    return &field->data.bool_type;
}


int64_t* Device_field_get_int(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_INT);
    return &field->data.int_type;
}


double* Device_field_get_float(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_FLOAT);
    return &field->data.float_type;
}


Real* Device_field_get_real(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_REAL);
    return &field->data.Real_type;
}


Reltime* Device_field_get_reltime(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_RELTIME);
    return &field->data.Reltime_type;
}


Envelope* Device_field_get_envelope(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_ENVELOPE);
    return field->data.Envelope_type;
}


Sample* Device_field_get_sample(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_WAVPACK);
    return field->data.Sample_type;
}


Sample_params* Device_field_get_sample_params(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_SAMPLE_PARAMS);
    return &field->data.Sample_params_type;
}


Sample_map* Device_field_get_sample_map(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_SAMPLE_MAP);
    return field->data.Sample_map_type;
}


void del_Device_field(Device_field* field)
{
    if (field == NULL)
    {
        return;
    }
    if (field->type == DEVICE_FIELD_ENVELOPE)
    {
        del_Envelope(field->data.Envelope_type);
    }
    if (field->type == DEVICE_FIELD_WAVPACK)
    {
        del_Sample(field->data.Sample_type);
    }
    if (field->type == DEVICE_FIELD_SAMPLE_MAP)
    {
        del_Sample_map(field->data.Sample_map_type);
    }
    xfree(field);
    return;
}


