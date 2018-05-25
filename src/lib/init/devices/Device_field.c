

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Device_field.h>

#include <debug/assert.h>
#include <init/devices/param_types/Wav.h>
#include <init/devices/param_types/Wavpack.h>
#include <memory.h>
#include <string/common.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ENVELOPE_NODES_MAX 256


typedef union
{
    bool bool_type;
    int64_t int_type;
    double float_type;
    Tstamp Tstamp_type;
    Envelope* Envelope_type;
    Sample* Sample_type;
    Sample_params Sample_params_type;
    Note_map* Note_map_type;
    Hit_map* Hit_map_type;
    Num_list* Num_list_type;
    Padsynth_params* Padsynth_params_type;
} Dev_fields;


struct Device_field
{
    char key[100];
    Device_field_type type;
    bool empty;
    Dev_fields data;
};


Device_field_type get_keyp_device_field_type(const char* keyp)
{
    rassert(keyp != NULL);

    // Find the last element
    const char* last_elem = keyp;
    const char* const last_sep = strrchr(keyp, '/');
    if (last_sep != NULL)
        last_elem = last_sep + 1;

    if (string_has_suffix(last_elem, ".json"))
    {
        if (string_has_prefix(last_elem, "p_b_"))
            return DEVICE_FIELD_BOOL;
        else if (string_has_prefix(last_elem, "p_i_"))
            return DEVICE_FIELD_INT;
        else if (string_has_prefix(last_elem, "p_f_"))
            return DEVICE_FIELD_FLOAT;
        else if (string_has_prefix(last_elem, "p_t_"))
            return DEVICE_FIELD_TSTAMP;
        else if (string_has_prefix(last_elem, "p_e_"))
            return DEVICE_FIELD_ENVELOPE;
        else if (string_has_prefix(last_elem, "p_sh_"))
            return DEVICE_FIELD_SAMPLE_PARAMS;
        else if (string_has_prefix(last_elem, "p_nm_"))
            return DEVICE_FIELD_NOTE_MAP;
        else if (string_has_prefix(last_elem, "p_hm_"))
            return DEVICE_FIELD_HIT_MAP;
        else if (string_has_prefix(last_elem, "p_ln_"))
            return DEVICE_FIELD_NUM_LIST;
        else if (string_has_prefix(last_elem, "p_ps_"))
            return DEVICE_FIELD_PADSYNTH_PARAMS;
    }
    else if (string_has_suffix(last_elem, ".wv"))
        return DEVICE_FIELD_WAVPACK;
    else if (string_has_suffix(last_elem, ".wav"))
        return DEVICE_FIELD_WAV;

    return DEVICE_FIELD_NONE;
}


Device_field* new_Device_field(const char* key, void* data)
{
    rassert(key != NULL);

    static const size_t sizes[DEVICE_FIELD_COUNT_] =
    {
        [DEVICE_FIELD_NONE]             = 0,
        [DEVICE_FIELD_BOOL]             = sizeof(bool),
        [DEVICE_FIELD_INT]              = sizeof(int64_t),
        [DEVICE_FIELD_FLOAT]            = sizeof(double),
        [DEVICE_FIELD_TSTAMP]           = sizeof(Tstamp),
        [DEVICE_FIELD_ENVELOPE]         = sizeof(Envelope*),
        [DEVICE_FIELD_SAMPLE_PARAMS]    = sizeof(Sample_params),
        [DEVICE_FIELD_NOTE_MAP]         = sizeof(Note_map*),
        [DEVICE_FIELD_HIT_MAP]          = sizeof(Hit_map*),
        [DEVICE_FIELD_NUM_LIST]         = sizeof(Num_list*),
        [DEVICE_FIELD_PADSYNTH_PARAMS]  = sizeof(Padsynth_params*),
        [DEVICE_FIELD_WAVPACK]          = sizeof(Sample*),
        [DEVICE_FIELD_WAV]              = sizeof(Sample*),
    };

    const Device_field_type type = get_keyp_device_field_type(key);
    rassert(type != DEVICE_FIELD_NONE);

    const size_t data_size = sizes[type];
    rassert(data_size > 0);

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


Device_field* new_Device_field_from_data(const char* key, int version, Streader* sr)
{
    rassert(key != NULL);
    rassert(version >= 0);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Device_field* field = new_Device_field(key, NULL);
    if (field == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for device field %s", key);
        return NULL;
    }

    if (!Device_field_change(field, version, sr))
    {
        del_Device_field(field);
        return NULL;
    }

    return field;
}


const char* Device_field_get_key(const Device_field* field)
{
    rassert(field != NULL);
    return field->key;
}


bool Device_field_change(Device_field* field, int version, Streader* sr)
{
    rassert(field != NULL);
    rassert(field->type != DEVICE_FIELD_NONE);
    rassert(version >= 0);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    const void* data = sr->str;

    // FIXME: Check for empty field before destroying dynamically allocated objects!

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
                env = new_Envelope(
                        ENVELOPE_NODES_MAX,
                        -INFINITY, INFINITY, 0,
                        -INFINITY, INFINITY, 0);
                if (env == NULL)
                    return false;

                if (!Envelope_read(env, sr))
                {
                    del_Envelope(env);
                    return false;
                }
            }

            rassert(!Streader_is_error_set(sr));
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

            rassert(!Streader_is_error_set(sr));
            if (field->data.Sample_type != NULL)
                del_Sample(field->data.Sample_type);

            field->data.Sample_type = sample;
        }
        break;

        case DEVICE_FIELD_WAV:
        {
            Sample* sample = NULL;
            if (data != NULL)
            {
                sample = new_Sample();
                if (sample == NULL)
                    return false;

                if (!Sample_parse_wav(sample, sr))
                {
                    del_Sample(sample);
                    return false;
                }
            }

            rassert(!Streader_is_error_set(sr));
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

        case DEVICE_FIELD_NOTE_MAP:
        {
            Note_map* map = new_Note_map_from_string(sr);
            if (map == NULL)
                return false;

            del_Note_map(field->data.Note_map_type);
            field->data.Note_map_type = map;
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
            if (Streader_is_error_set(sr))
                return false;

            del_Num_list(field->data.Num_list_type);
            field->data.Num_list_type = nl;
        }
        break;

        case DEVICE_FIELD_PADSYNTH_PARAMS:
        {
            Padsynth_params* pp = new_Padsynth_params(sr);
            if (pp == NULL)
                return false;

            del_Padsynth_params(field->data.Padsynth_params_type);
            field->data.Padsynth_params_type = pp;
        }
        break;

        default:
            rassert(false);
    }

    if (Streader_is_error_set(sr))
        return false;

    field->empty = (data == NULL);

    return true;
}


int Device_field_cmp(const Device_field* field1, const Device_field* field2)
{
    rassert(field1 != NULL);
    rassert(field1->key != NULL);
    rassert(field2 != NULL);
    rassert(field2->key != NULL);

    return strcmp(field1->key, field2->key);
}


void Device_field_set_empty(Device_field* field, bool empty)
{
    rassert(field != NULL);
    rassert(field->type != DEVICE_FIELD_ENVELOPE);
    rassert(field->type != DEVICE_FIELD_WAVPACK);
    rassert(field->type != DEVICE_FIELD_WAV);
    rassert(field->type != DEVICE_FIELD_NOTE_MAP);
    rassert(field->type != DEVICE_FIELD_HIT_MAP);

    field->empty = empty;

    return;
}


bool Device_field_get_empty(const Device_field* field)
{
    rassert(field != NULL);
    return field->empty;
}


#if 0
bool Device_field_modify(Device_field* field, void* data)
{
    rassert(field != NULL);
    rassert(field->type != DEVICE_FIELD_ENVELOPE);
    rassert(field->type != DEVICE_FIELD_WAVPACK);
    rassert(field->type != DEVICE_FIELD_WAV);
    rassert(field->type != DEVICE_FIELD_NOTE_MAP);
    rassert(field->type != DEVICE_FIELD_HIT_MAP);
    rassert(data != NULL);

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
            rassert(false);
    }

    Device_field_set_empty(field, false);

    return true;
}
#endif


const bool* Device_field_get_bool(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_BOOL);

    if (field->empty)
        return NULL;

    return &field->data.bool_type;
}


const int64_t* Device_field_get_int(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_INT);

    if (field->empty)
        return NULL;

    return &field->data.int_type;
}


const double* Device_field_get_float(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_FLOAT);

    if (field->empty)
        return NULL;

    return &field->data.float_type;
}


const Tstamp* Device_field_get_tstamp(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_TSTAMP);

    if (field->empty)
        return NULL;

    return &field->data.Tstamp_type;
}


const Envelope* Device_field_get_envelope(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_ENVELOPE);

    if (field->empty)
        return NULL;

    return field->data.Envelope_type;
}


const Sample* Device_field_get_sample(const Device_field* field)
{
    rassert(field != NULL);
    rassert((field->type == DEVICE_FIELD_WAVPACK) || (field->type == DEVICE_FIELD_WAV));

    if (field->empty)
        return NULL;

    return field->data.Sample_type;
}


const Sample_params* Device_field_get_sample_params(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_SAMPLE_PARAMS);

    if (field->empty)
        return NULL;

    return &field->data.Sample_params_type;
}


const Note_map* Device_field_get_note_map(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_NOTE_MAP);

    if (field->empty)
        return NULL;

    return field->data.Note_map_type;
}


const Hit_map* Device_field_get_hit_map(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_HIT_MAP);

    if (field->empty)
        return NULL;

    return field->data.Hit_map_type;
}


const Num_list* Device_field_get_num_list(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_NUM_LIST);

    if (field->empty)
        return NULL;

    return field->data.Num_list_type;
}


const Padsynth_params* Device_field_get_padsynth_params(const Device_field* field)
{
    rassert(field != NULL);
    rassert(field->type == DEVICE_FIELD_PADSYNTH_PARAMS);

    if (field->empty)
        return NULL;

    return field->data.Padsynth_params_type;
}


void del_Device_field(Device_field* field)
{
    if (field == NULL)
        return;

    // FIXME: Check for empty field before destroying dynamically allocated objects!

    if (field->type == DEVICE_FIELD_ENVELOPE)
        del_Envelope(field->data.Envelope_type);
    else if ((field->type == DEVICE_FIELD_WAVPACK) || (field->type == DEVICE_FIELD_WAV))
        del_Sample(field->data.Sample_type);
    else if (field->type == DEVICE_FIELD_NOTE_MAP)
        del_Note_map(field->data.Note_map_type);
    else if (field->type == DEVICE_FIELD_HIT_MAP)
        del_Hit_map(field->data.Hit_map_type);
    else if (field->type == DEVICE_FIELD_NUM_LIST)
        del_Num_list(field->data.Num_list_type);
    else if (field->type == DEVICE_FIELD_PADSYNTH_PARAMS)
        del_Padsynth_params(field->data.Padsynth_params_type);

    memory_free(field);

    return;
}


