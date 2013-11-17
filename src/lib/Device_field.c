

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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

#include <Device_field.h>
#include <generators/File_wavpack.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


typedef union
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Real Real_type;
    Tstamp Tstamp_type;
    Envelope* Envelope_type;
    Sample* Sample_type;
    Sample_params Sample_params_type;
    Sample_map* Sample_map_type;
    Hit_map* Hit_map_type;
    Num_list* Num_list_type;
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
        type = DEVICE_FIELD_TSTAMP;
        data_size = sizeof(Tstamp);
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
    else if (string_has_suffix(key, ".jsonhm"))
    {
        type = DEVICE_FIELD_HIT_MAP;
        data_size = sizeof(Hit_map*);
    }
    else if (string_has_suffix(key, ".jsonln"))
    {
        type = DEVICE_FIELD_NUM_LIST;
        data_size = sizeof(Num_list*);
    }
    else
        assert(false);

    Device_field* field = memory_alloc_item(Device_field);
    if (field == NULL)
        return NULL;

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


Device_field* new_Device_field_from_data(const char* key, Streader* sr)
{
    assert(key != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Device_field* field = new_Device_field(key, NULL);
    if (field == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for device field %s", key);
        return NULL;
    }

    if (!Device_field_change(field, sr))
    {
        del_Device_field(field);
        return NULL;
    }

    return field;
}


const char* Device_field_get_key(const Device_field* field)
{
    assert(field != NULL);
    return field->key;
}


bool Device_field_change(Device_field* field, Streader* sr)
{
    assert(field != NULL);
    assert(field->type != DEVICE_FIELD_NONE);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    const void* data = sr->str;

    switch (field->type)
    {
        case DEVICE_FIELD_BOOL:
        {
            if (data != NULL)
                Streader_read_bool(sr, &field->data.bool_type);
        }
        break;

        case DEVICE_FIELD_INT:
        {
            if (data != NULL)
                Streader_read_int(sr, &field->data.int_type);
        }
        break;

        case DEVICE_FIELD_FLOAT:
        {
            if (data != NULL)
                Streader_read_float(sr, &field->data.float_type);
        }
        break;

        case DEVICE_FIELD_REAL:
        {
            if (data != NULL)
            {
                assert(false); // TODO: implement
            }
        }
        break;

        case DEVICE_FIELD_TSTAMP:
        {
            if (data != NULL)
                Streader_read_tstamp(sr, &field->data.Tstamp_type);
        }
        break;

        case DEVICE_FIELD_ENVELOPE:
        {
            Envelope* env = NULL;
            if (data != NULL)
            {
                env = new_Envelope(32, -INFINITY, INFINITY, 0,
                                       -INFINITY, INFINITY, 0);
                if (env == NULL)
                    return false;

                if (!Envelope_read(env, sr))
                {
                    del_Envelope(env);
                    return false;
                }
            }

            assert(!Streader_is_error_set(sr));
            if (field->data.Envelope_type != NULL)
                del_Envelope(field->data.Envelope_type);

            field->data.Envelope_type = env;
        }
        break;

        case DEVICE_FIELD_WAVPACK:
        {
            Sample* sample = NULL;
            if (data != NULL)
            {
                sample = new_Sample();
                if (sample == NULL)
                    return false;

                if (!Sample_parse_wavpack(sample, sr))
                {
                    del_Sample(sample);
                    return false;
                }
            }

            assert(!Streader_is_error_set(sr));
            if (field->data.Sample_type != NULL)
                del_Sample(field->data.Sample_type);

            field->data.Sample_type = sample;
        }
        break;

        case DEVICE_FIELD_SAMPLE_PARAMS:
        {
            if (!Sample_params_parse(&field->data.Sample_params_type, sr))
                return false;
        }
        break;

        case DEVICE_FIELD_SAMPLE_MAP:
        {
            Sample_map* map = new_Sample_map_from_string(sr);
            if (map == NULL)
                return false;

            del_Sample_map(field->data.Sample_map_type);
            field->data.Sample_map_type = map;
        }
        break;

        case DEVICE_FIELD_HIT_MAP:
        {
            Hit_map* map = new_Hit_map_from_string(sr);
            if (map == NULL)
                return false;

            del_Hit_map(field->data.Hit_map_type);
            field->data.Hit_map_type = map;
        }
        break;

        case DEVICE_FIELD_NUM_LIST:
        {
            Num_list* nl = new_Num_list_from_string(sr);
            if (nl == NULL)
                return false;

            del_Num_list(field->data.Num_list_type);
            field->data.Num_list_type = nl;
        }
        break;

        default:
            assert(false);
    }

    if (Streader_is_error_set(sr))
        return false;

    field->empty = (data == NULL);

    return true;
}


int Device_field_cmp(const Device_field* field1, const Device_field* field2)
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
        field->empty = empty;

    return;
}


bool Device_field_get_empty(Device_field* field)
{
    assert(field != NULL);
    return field->empty;
}


bool Device_field_modify(Device_field* field, void* data)
{
    assert(field != NULL);
    assert(field->type != DEVICE_FIELD_ENVELOPE);
    assert(field->type != DEVICE_FIELD_WAVPACK);
    assert(field->type != DEVICE_FIELD_SAMPLE_MAP);
    assert(field->type != DEVICE_FIELD_HIT_MAP);
    assert(data != NULL);

    switch (field->type)
    {
        case DEVICE_FIELD_BOOL:
            memcpy(&field->data.bool_type, data, sizeof(bool));
            break;

        case DEVICE_FIELD_INT:
            memcpy(&field->data.int_type, data, sizeof(int64_t));
            break;

        case DEVICE_FIELD_FLOAT:
            memcpy(&field->data.float_type, data, sizeof(double));
            break;

        case DEVICE_FIELD_TSTAMP:
            memcpy(&field->data.Tstamp_type, data, sizeof(Tstamp));
            break;

        default:
            assert(false);
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


Tstamp* Device_field_get_tstamp(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_TSTAMP);

    return &field->data.Tstamp_type;
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


Hit_map* Device_field_get_hit_map(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_HIT_MAP);

    return field->data.Hit_map_type;
}


Num_list* Device_field_get_num_list(Device_field* field)
{
    assert(field != NULL);
    assert(field->type == DEVICE_FIELD_NUM_LIST);

    return field->data.Num_list_type;
}


void del_Device_field(Device_field* field)
{
    if (field == NULL)
        return;

    if (field->type == DEVICE_FIELD_ENVELOPE)
        del_Envelope(field->data.Envelope_type);
    else if (field->type == DEVICE_FIELD_WAVPACK)
        del_Sample(field->data.Sample_type);
    else if (field->type == DEVICE_FIELD_SAMPLE_MAP)
        del_Sample_map(field->data.Sample_map_type);
    else if (field->type == DEVICE_FIELD_HIT_MAP)
        del_Hit_map(field->data.Hit_map_type);
    else if (field->type == DEVICE_FIELD_NUM_LIST)
        del_Num_list(field->data.Num_list_type);

    memory_free(field);

    return;
}


