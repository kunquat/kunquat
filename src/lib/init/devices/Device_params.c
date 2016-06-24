

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Device_params.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <init/devices/Device_field.h>
#include <memory.h>
#include <string/common.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Device_params
{
    AAtree* implement;  ///< The implementation part of the device.
    AAtree* config;     ///< The configuration part of the device.
};


Device_params_iter* Device_params_iter_init(
        Device_params_iter* iter, const Device_params* dparams)
{
    assert(iter != NULL);
    assert(dparams != NULL);

    // Init tree iterators
    AAiter_init(&iter->impl_iter, dparams->implement);
    AAiter_init(&iter->config_iter, dparams->config);

    // Retrieve first keys of each tree
    const Device_field* impl_field = AAiter_get_at_least(&iter->impl_iter, "");
    iter->next_impl_key = (impl_field != NULL) ?
        Device_field_get_key(impl_field) : NULL;

    const Device_field* config_field = AAiter_get_at_least(
            &iter->config_iter, "");
    iter->next_config_key = (config_field != NULL) ?
        Device_field_get_key(config_field) : NULL;

    return iter;
}


const char* Device_params_iter_get_next_key(Device_params_iter* iter)
{
    assert(iter != NULL);

    // Get the next key to be returned
    const char* ret_key = iter->next_impl_key;
    if (ret_key == NULL ||
            (iter->next_config_key != NULL &&
             strcmp(iter->next_config_key, ret_key) < 0))
        ret_key = iter->next_config_key;

    assert(!(ret_key == NULL) ||
            (iter->next_impl_key == NULL && iter->next_config_key == NULL));
    if (ret_key == NULL)
        return NULL; // end reached

    assert(iter->next_impl_key != NULL || iter->next_config_key != NULL);

    // Iterate the tree that is behind the other tree
    if (iter->next_config_key == NULL ||
            (iter->next_impl_key != NULL &&
             strcmp(iter->next_impl_key, iter->next_config_key) < 0))
    {
        const Device_field* impl_field = AAiter_get_next(&iter->impl_iter);
        iter->next_impl_key = (impl_field != NULL) ?
            Device_field_get_key(impl_field) : NULL;
        return ret_key;
    }
    else if (iter->next_impl_key == NULL ||
            (iter->next_config_key != NULL &&
             strcmp(iter->next_config_key, iter->next_impl_key) < 0))
    {
        const Device_field* config_field = AAiter_get_next(&iter->config_iter);
        iter->next_config_key = (config_field != NULL) ?
            Device_field_get_key(config_field) : NULL;
        return ret_key;
    }

    // Both tree iterators point to the same key, move both forwards
    assert(iter->next_impl_key != NULL && iter->next_config_key != NULL);
    assert(string_eq(iter->next_impl_key, iter->next_config_key));

    const Device_field* impl_field = AAiter_get_next(&iter->impl_iter);
    iter->next_impl_key = (impl_field != NULL) ?
        Device_field_get_key(impl_field) : NULL;

    const Device_field* config_field = AAiter_get_next(&iter->config_iter);
    iter->next_config_key = (config_field != NULL) ?
        Device_field_get_key(config_field) : NULL;

    return ret_key;
}


bool key_is_device_param(const char* key)
{
    assert(key != NULL);

    if (key_is_text_device_param(key))
        return true;

    const Device_field_type type = get_keyp_device_field_type(key);
    return (type == DEVICE_FIELD_WAVPACK) ||
        (type == DEVICE_FIELD_WAV) ||
        (type == DEVICE_FIELD_VORBIS);
}


bool key_is_real_time_device_param(const char* key)
{
    assert(key != NULL);

    const Device_field_type type = get_keyp_device_field_type(key);
    return type == DEVICE_FIELD_BOOL ||
        type == DEVICE_FIELD_INT ||
        type == DEVICE_FIELD_FLOAT ||
        type == DEVICE_FIELD_TSTAMP;
}


bool key_is_text_device_param(const char* key)
{
    assert(key != NULL);

    if (key_is_real_time_device_param(key))
        return true;

    const Device_field_type type = get_keyp_device_field_type(key);
    return type == DEVICE_FIELD_ENVELOPE ||
        type == DEVICE_FIELD_NOTE_MAP ||
        type == DEVICE_FIELD_HIT_MAP ||
        type == DEVICE_FIELD_SAMPLE_PARAMS ||
        type == DEVICE_FIELD_NUM_LIST ||
        type == DEVICE_FIELD_PADSYNTH_PARAMS;
}


Device_params* new_Device_params(void)
{
    Device_params* params = memory_alloc_item(Device_params);
    if (params == NULL)
        return NULL;

    params->implement = NULL;
    params->config = NULL;

    params->implement = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Device_field);
    params->config = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Device_field);
    if (params->implement == NULL || params->config == NULL)
    {
        del_Device_params(params);
        return NULL;
    }

    return params;
}


bool Device_params_parse_value(Device_params* params, const char* key, Streader* sr)
{
    assert(params != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(key_is_device_param(key));
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    AAtree* tree = NULL;
    if (string_has_prefix(key, "i/"))
    {
        tree = params->implement;
        key = key + 2;
    }
    else if (string_has_prefix(key, "c/"))
    {
        tree = params->config;
        key = key + 2;
    }
    else
    {
        assert(false);
    }

    assert(tree != NULL);
    Device_field* field = AAtree_get_exact(tree, key);
    bool success = true;
    if (field != NULL)
    {
        success = Device_field_change(field, sr);
    }
    else
    {
        field = new_Device_field_from_data(key, sr);
        if (field == NULL)
            return false;

        if (!AAtree_ins(tree, field))
        {
            del_Device_field(field);
            Streader_set_memory_error(
                    sr, "Could not allocate memory for device key %s", key);
            return false;
        }
    }

    return success;
}


#define get_of_type(params, key, ftype)                                      \
    if (true)                                                                \
    {                                                                        \
        Device_field* field = AAtree_get_exact(params->config, (key));       \
        if (field != NULL)                                                   \
            return Device_field_get_ ## ftype(field);                        \
                                                                             \
        field = AAtree_get_exact(params->implement, (key));                  \
        if (field != NULL)                                                   \
            return Device_field_get_ ## ftype(field);                        \
    }                                                                        \
    else (void)0

const bool* Device_params_get_bool(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_BOOL)
        return NULL;

    get_of_type(params, key, bool);

    return NULL;
}


const int64_t* Device_params_get_int(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_INT)
        return NULL;

    get_of_type(params, key, int);

    return NULL;
}


const double* Device_params_get_float(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_FLOAT)
        return NULL;

    get_of_type(params, key, float);

    return NULL;
}


const Tstamp* Device_params_get_tstamp(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_TSTAMP)
        return NULL;

    get_of_type(params, key, tstamp);

    return NULL;
}


const Envelope* Device_params_get_envelope(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_ENVELOPE)
        return NULL;

    get_of_type(params, key, envelope);

    return NULL;
}


const Sample* Device_params_get_sample(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    const Device_field_type type = get_keyp_device_field_type(key);
    if ((type != DEVICE_FIELD_WAVPACK) && (type != DEVICE_FIELD_WAV))
        return NULL;

    get_of_type(params, key, sample);

    return NULL;
}


const Sample_params* Device_params_get_sample_params(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_SAMPLE_PARAMS)
        return NULL;

    get_of_type(params, key, sample_params);

    return NULL;
}


const Note_map* Device_params_get_note_map(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_NOTE_MAP)
        return NULL;

    get_of_type(params, key, note_map);

    return NULL;
}


const Hit_map* Device_params_get_hit_map(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_HIT_MAP)
        return NULL;

    get_of_type(params, key, hit_map);

    return NULL;
}


const Num_list* Device_params_get_num_list(const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_NUM_LIST)
        return NULL;

    get_of_type(params, key, num_list);

    return NULL;
}


const Padsynth_params* Device_params_get_padsynth_params(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_PADSYNTH_PARAMS)
        return NULL;

    get_of_type(params, key, padsynth_params);

    return NULL;
}

#undef get_of_type


void del_Device_params(Device_params* params)
{
    if (params == NULL)
        return;

    del_AAtree(params->implement);
    del_AAtree(params->config);
    memory_free(params);

    return;
}


