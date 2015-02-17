

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <math.h>
#include <stdio.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <devices/Device_event_keys.h>
#include <devices/Device_field.h>
#include <devices/Device_params.h>
#include <memory.h>
#include <string/common.h>


#if 0
typedef struct Event_name_to_param
{
    char name[129];
    char param[100];
} Event_name_to_param;


Event_name_to_param* new_Event_name_to_param(const char key[],
                                             const char param[])
{
    assert(key != NULL);
    assert(param != NULL);
    Event_name_to_param* e = xalloc(Event_name_to_param);
    if (e == NULL)
    {
        return NULL;
    }
    strncpy(e->name, key, 128);
    e->name[128] = '\0';
    strncpy(e->param, param, 99);
    e->param[99] = '\0';
    return e;
}
#endif


#if 0
typedef struct Slow_sync_info
{
    char key[100];
    bool sync_needed;
} Slow_sync_info;


static Slow_sync_info* new_Slow_sync_info(const char* key);

static void del_Slow_sync_info(Slow_sync_info* info);


static Slow_sync_info* new_Slow_sync_info(const char* key)
{
    assert(key != NULL);
    Slow_sync_info* info = memory_alloc_item(Slow_sync_info);
    if (info == NULL)
    {
        return NULL;
    }
    strncpy(info->key, key, 100);
    info->key[99] = '\0';
    info->sync_needed = true;
    return info;
}


static void del_Slow_sync_info(Slow_sync_info* info)
{
    memory_free(info);
}
#endif


struct Device_params
{
    AAtree* implement;       ///< The implementation part of the device.
    AAtree* config;          ///< The configuration part of the device.
#if 0
    AAtree* slow_sync;       ///< Keys that require explicit synchronisation.
    AAiter* slow_sync_iter;  ///< Iterator for slow_sync.
    bool slow_sync_needed;   ///< Whether any slow-sync keys have changed.
    bool slow_sync_keys_requested;
#endif
//    AAtree* event_names;    ///< A mapping from event names to parameters.
};


Device_params_iter* Device_params_iter_init(
        Device_params_iter* iter,
        const Device_params* dparams)
{
    assert(iter != NULL);
    assert(dparams != NULL);

    // Init tree iterators
    AAiter_change_tree(&iter->impl_iter, dparams->implement);
    AAiter_change_tree(&iter->config_iter, dparams->config);

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
        type == DEVICE_FIELD_NUM_LIST;
}


Device_params* new_Device_params(void)
{
    Device_params* params = memory_alloc_item(Device_params);
    if (params == NULL)
        return NULL;

    params->implement = NULL;
    params->config = NULL;
#if 0
    params->slow_sync = NULL;
    params->slow_sync_iter = NULL;
    params->slow_sync_needed = false;
    params->slow_sync_keys_requested = false;
#endif

    params->implement = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Device_field);
    params->config = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Device_field);
#if 0
    params->slow_sync = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Slow_sync_info);
    params->slow_sync_iter = new_AAiter(params->slow_sync);
#endif
//    params->event_names = new_AAtree((int (*)(const void*, const void*))strcmp,
//                                     memory_free);
    if (params->implement == NULL || params->config == NULL)
//            params->slow_sync == NULL || params->slow_sync_iter == NULL)
//             || params->event_names == NULL)
    {
        del_Device_params(params);
        return NULL;
    }

    return params;
}


#if 0
bool Device_params_set_key(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (AAtree_get_exact(params->event_data, key) != NULL)
    {
        return true;
    }
    Device_field* field = new_Device_field(key, NULL);
    if (field == NULL)
    {
        return false;
    }
    if (!AAtree_ins(params->event_data, field))
    {
        del_Device_field(field);
        return false;
    }
    return true;
}
#endif


#if 0
bool Device_params_set_slow_sync(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (AAtree_contains(params->slow_sync, key))
    {
        params->slow_sync_needed = true;
        return true;
    }
    Slow_sync_info* info = new_Slow_sync_info(key);
    if (info == NULL || !AAtree_ins(params->slow_sync, info))
    {
        del_Slow_sync_info(info);
        return false;
    }
    params->slow_sync_needed = true;
    return true;
}


void Device_params_clear_slow_sync(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    del_Slow_sync_info(AAtree_remove(params->slow_sync, key));
    return;
}


bool Device_params_need_sync(Device_params* params)
{
    assert(params != NULL);
    return params->slow_sync_needed;
}


const char* Device_params_get_slow_sync_key(Device_params* params)
{
    assert(params != NULL);
    Slow_sync_info* info = NULL;
    if (!params->slow_sync_keys_requested)
    {
        params->slow_sync_keys_requested = true;
        info = AAiter_get_at_least(params->slow_sync_iter, "");
    }
    else
    {
        info = AAiter_get_next(params->slow_sync_iter);
    }
    while (info != NULL && !info->sync_needed)
    {
        info = AAiter_get_next(params->slow_sync_iter);
    }
    return info != NULL ? info->key : NULL;
}


void Device_params_synchronised(Device_params* params)
{
    assert(params != NULL);
    Slow_sync_info* info = AAiter_get_at_least(params->slow_sync_iter, "");
    while (info != NULL)
    {
        info->sync_needed = false;
        info = AAiter_get_next(params->slow_sync_iter);
    }
    params->slow_sync_needed = false;
    params->slow_sync_keys_requested = false;
    return;
}
#endif


#if 0
#define clean_if_fail()                                 \
    if (true)                                           \
    {                                                   \
        if (state->error)                               \
        {                                               \
            del_AAtree(params->event_data);             \
            params->event_data = old_data;              \
            AAiter_change_tree(params->event_data_iter, \
                               params->event_data);     \
/*            del_AAtree(params->event_names);            \
            params->event_names = old_names; */           \
            return false;                               \
        }                                               \
    } else (void)0

bool Device_params_parse_events(Device_params* params,
                                Device_event_type type,
                                Player* player,
                                char* str,
                                Read_state* state)
{
    assert(params != NULL);
    assert(player != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    AAtree* old_data = params->event_data;
    params->event_data = new_AAtree(
            (int (*)(const void*, const void*))Device_field_cmp,
            (void (*)(void*))del_Device_field);
    if (params->event_data == NULL)
    {
        params->event_data = old_data;
        return false;
    }
    AAiter_change_tree(params->event_data_iter, params->event_data);
#if 0
    AAtree* old_names = params->event_names;
    params->event_names = new_AAtree((int (*)(const void*, const void*))strcmp,
                                     memory_free);
    if (params->event_names == NULL)
    {
        del_AAtree(params->event_data);
        params->event_data = old_data;
        params->event_names = old_names;
        return false;
    }
#endif
    if (str == NULL)
    {
        del_AAtree(old_data);
//        del_AAtree(old_names);
        return true;
    }
    str = read_const_char(str, '[', state);
    clean_if_fail();
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        del_AAtree(old_data);
//        del_AAtree(old_names);
        return true;
    }
    Read_state_clear_error(state);

    bool expect_entry = true;
    while (expect_entry)
    {
        str = read_const_char(str, '[', state);
        clean_if_fail();
//        char name[129] = { '\0' };
//        str = read_string(str, name, 128, state);
//        str = read_const_char(str, ',', state);
        bool channel_level = false;
        str = read_bool(str, &channel_level, state);
        str = read_const_char(str, ',', state);
        char param[100] = { '\0' };
        str = read_string(str, param, 99, state);
        clean_if_fail();

        if (!channel_level) // generator level
        {
            if (!key_is_real_time_device_param(param))
            {
                Read_state_set_error(state, "Key %s cannot be modified"
                                     " through events", param);
                clean_if_fail();
            }
#if 0
            Event_name_to_param* e = new_Event_name_to_param(name, param);
            if (e == NULL || !AAtree_ins(params->event_names, e))
            {
                del_AAtree(params->event_names);
                params->event_names = old_names;
                del_AAtree(params->event_data);
                params->event_data = old_data;
                return false;
            }
#endif
            Device_field* field = new_Device_field(param, NULL);
            if (field == NULL || (/*Device_field_set_event_control(field, true),*/
                                  !AAtree_ins(params->event_data, field)))
            {
//                del_AAtree(params->event_names);
//                params->event_names = old_names;
                del_AAtree(params->event_data);
                params->event_data = old_data;
                AAiter_change_tree(params->event_data_iter,
                                   params->event_data);
                return false;
            }
        }
        else // channel level
        {
            if (!key_is_real_time_device_param(param))
            {
                Read_state_set_error(state, "Key %s cannot be modified"
                                     " through events", param);
                clean_if_fail();
            }
            if (type == DEVICE_EVENT_TYPE_GENERATOR)
            {
                if (!Player_add_channel_gen_state_key(player, param))
                {
//                    del_AAtree(params->event_names);
//                    params->event_names = old_names;
                    del_AAtree(params->event_data);
                    params->event_data = old_data;
                    AAiter_change_tree(params->event_data_iter,
                                       params->event_data);
                    return false;
                }
            }
            else if (type == DEVICE_EVENT_TYPE_DSP)
            {
                Read_state_set_error(state, "DSP events are not"
                                            " channel-specific");
                clean_if_fail();
            }
            else
            {
                assert(false);
            }
        }

        str = read_const_char(str, ']', state);
        clean_if_fail();
        check_next(str, state, expect_entry);
    }
    str = read_const_char(str, ']', state);
    clean_if_fail();

    del_AAtree(old_data);
//    del_AAtree(old_names);
    return true;
}

#undef clean_if_fail
#endif


bool Device_params_parse_value(
        Device_params* params, const char* key, Streader* sr)
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

#if 0
    if (success && AAtree_contains(params->slow_sync, key))
    {
        Slow_sync_info* info = AAtree_get_exact(params->slow_sync, key);
        assert(info != NULL);
        info->sync_needed = true;
        params->slow_sync_needed = true;
    }
#endif

    return success;
}


#if 0
bool Device_params_modify_value(Device_params* params,
                                const char* key,
                                void* data)
{
    assert(params != NULL);
    assert(key != NULL);
    assert(key_is_real_time_device_param(key));
    assert(data != NULL);
    if (AAtree_contains(params->slow_sync, key))
    {
        return false;
    }
    Device_field* field = AAtree_get_exact(params->event_data, key);
    if (field == NULL)
    {
        return false;
    }
    return Device_field_modify(field, data);
}


void Device_params_reset(Device_params* params)
{
    assert(params != NULL);
    AAiter_change_tree(params->event_data_iter, params->event_data);
    Device_field* field = AAiter_get_at_least(params->event_data_iter, "");
    while (field != NULL)
    {
        Device_field_set_empty(field, true);
        field = AAiter_get_next(params->event_data_iter);
    }
    return;
}
#endif


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

const bool* Device_params_get_bool(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_BOOL)
        return NULL;

    get_of_type(params, key, bool);

    return NULL;
}


const int64_t* Device_params_get_int(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_INT)
        return NULL;

    get_of_type(params, key, int);

    return NULL;
}


const double* Device_params_get_float(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_FLOAT)
        return NULL;

    get_of_type(params, key, float);

    return NULL;
}


const Real* Device_params_get_real(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_REAL)
        return NULL;

    get_of_type(params, key, real);

    return NULL;
}


const Tstamp* Device_params_get_tstamp(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_TSTAMP)
        return NULL;

    get_of_type(params, key, tstamp);

    return NULL;
}


const Envelope* Device_params_get_envelope(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_ENVELOPE)
        return NULL;

    get_of_type(params, key, envelope);

    return NULL;
}


const Sample* Device_params_get_sample(
        const Device_params* params, const char* key)
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


const Note_map* Device_params_get_note_map(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_NOTE_MAP)
        return NULL;

    get_of_type(params, key, note_map);

    return NULL;
}


const Hit_map* Device_params_get_hit_map(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_HIT_MAP)
        return NULL;

    get_of_type(params, key, hit_map);

    return NULL;
}


const Num_list* Device_params_get_num_list(
        const Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);

    if (get_keyp_device_field_type(key) != DEVICE_FIELD_NUM_LIST)
        return NULL;

    get_of_type(params, key, num_list);

    return NULL;
}

#undef get_of_type


void del_Device_params(Device_params* params)
{
    if (params == NULL)
        return;

    del_AAtree(params->implement);
    del_AAtree(params->config);
    //del_AAtree(params->slow_sync);
    //del_AAiter(params->slow_sync_iter);
#if 0
    del_AAtree(params->event_names);
#endif
    memory_free(params);

    return;
}


