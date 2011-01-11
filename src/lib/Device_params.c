

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
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <AAtree.h>
#include <Device_event_keys.h>
#include <Device_field.h>
#include <Device_params.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


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
    Slow_sync_info* info = xalloc(Slow_sync_info);
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
    xfree(info);
}


struct Device_params
{
    AAtree* implement;       ///< The implementation part of the generator.
    AAtree* config;          ///< The configuration part of the generator.
    AAtree* event_data;      ///< The playback state of the parameters.
    AAiter* event_data_iter; ///< Iterator used by Device_params_reset.
    AAtree* slow_sync;       ///< Keys that require explicit synchronisation.
    AAiter* slow_sync_iter;  ///< Iterator for slow_sync.
    bool slow_sync_needed;   ///< Whether any slow-sync keys have changed.
    bool slow_sync_keys_requested;
//    AAtree* event_names;    ///< A mapping from event names to parameters.
};


bool key_is_device_param(const char* key)
{
    assert(key != NULL);
    return key_is_real_time_device_param(key) ||
           key_is_text_device_param(key) ||
           string_has_suffix(key, ".wv") ||
           string_has_suffix(key, ".ogg");
}


bool key_is_real_time_device_param(const char* key)
{
    assert(key != NULL);
    return string_has_suffix(key, ".jsonb") ||
           string_has_suffix(key, ".jsoni") ||
           string_has_suffix(key, ".jsonf") ||
           string_has_suffix(key, ".jsonr") ||
           string_has_suffix(key, ".jsont");
}


bool key_is_text_device_param(const char* key)
{
    assert(key != NULL);
    return key_is_real_time_device_param(key) ||
           string_has_suffix(key, ".jsone") ||
           string_has_suffix(key, ".jsonsm") ||
           string_has_suffix(key, ".jsonsh");
}


Device_params* new_Device_params(void)
{
    Device_params* params = xalloc(Device_params);
    if (params == NULL)
    {
        return NULL;
    }
    params->implement = NULL;
    params->config = NULL;
    params->event_data = NULL;
    params->event_data_iter = NULL;
    params->slow_sync = NULL;
    params->slow_sync_iter = NULL;
    params->slow_sync_needed = false;
    params->slow_sync_keys_requested = false;

    params->implement = new_AAtree((int (*)(const void*, const void*))strcmp,
                                   (void (*)(void*))del_Device_field);
    params->config = new_AAtree((int (*)(const void*, const void*))strcmp,
                                (void (*)(void*))del_Device_field);
    params->event_data = new_AAtree((int (*)(const void*, const void*))strcmp,
                                    (void (*)(void*))del_Device_field);
    params->event_data_iter = new_AAiter(params->event_data);
    params->slow_sync = new_AAtree((int (*)(const void*, const void*))strcmp,
                                   (void (*)(void*))del_Slow_sync_info);
    params->slow_sync_iter = new_AAiter(params->slow_sync);
//    params->event_names = new_AAtree((int (*)(const void*, const void*))strcmp,
//                                     free);
    if (params->implement == NULL || params->config == NULL ||
            params->event_data == NULL || params->event_data_iter == NULL ||
            params->slow_sync == NULL || params->slow_sync_iter == NULL)
//             || params->event_names == NULL)
    {
        del_Device_params(params);
        return NULL;
    }
    return params;
}


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
        info = AAiter_get(params->slow_sync_iter, "");
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
    Slow_sync_info* info = AAiter_get(params->slow_sync_iter, "");
    while (info != NULL)
    {
        info->sync_needed = false;
        info = AAiter_get_next(params->slow_sync_iter);
    }
    params->slow_sync_needed = false;
    params->slow_sync_keys_requested = false;
    return;
}


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
                                Event_handler* eh,
                                char* str,
                                Read_state* state)
{
    assert(params != NULL);
    assert(eh != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    AAtree* old_data = params->event_data;
    params->event_data = new_AAtree((int (*)(const void*,
                                             const void*))Device_field_cmp,
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
                                     free);
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
                if (!Event_handler_add_channel_gen_state_key(eh, param))
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


bool Device_params_parse_value(Device_params* params,
                               const char* key,
                               void* data,
                               long length,
                               Read_state* state)
{
    assert(params != NULL);
    assert(key != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(key_is_device_param(key));
    if (state->error)
    {
        return false;
    }
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
        success = Device_field_change(field, data, length, state);
    }
    else
    {
        field = new_Device_field_from_data(key, data, length, state);
        if (field == NULL)
        {
            return false;
        }
        if (!AAtree_ins(tree, field))
        {
            del_Device_field(field);
            return false;
        }
    }
    if (success && AAtree_contains(params->slow_sync, key))
    {
        Slow_sync_info* info = AAtree_get_exact(params->slow_sync, key);
        assert(info != NULL);
        info->sync_needed = true;
        params->slow_sync_needed = true;
    }
    return success;
}


bool Device_params_modify_value(Device_params* params,
                                const char* key,
                                char* str)
{
    assert(params != NULL);
    assert(key != NULL);
    assert(key_is_real_time_device_param(key));
    assert(str != NULL);
    if (AAtree_contains(params->slow_sync, key))
    {
        return false;
    }
    Device_field* field = AAtree_get_exact(params->event_data, key);
    if (field == NULL)
    {
        return false;
    }
    return Device_field_modify(field, str);
}


void Device_params_reset(Device_params* params)
{
    assert(params != NULL);
    AAiter_change_tree(params->event_data_iter, params->event_data);
    Device_field* field = AAiter_get(params->event_data_iter, "");
    while (field != NULL)
    {
        Device_field_set_empty(field, true);
        field = AAiter_get_next(params->event_data_iter);
    }
    return;
}


#define get_of_type(params, key, ftype)                                      \
    if (true)                                                                \
    {                                                                        \
        Device_field* field = AAtree_get_exact((params)->event_data, (key)); \
        if (field != NULL && !Device_field_get_empty(field))                 \
        {                                                                    \
            return Device_field_get_ ## ftype(field);                        \
        }                                                                    \
        field = AAtree_get_exact(params->config, (key));                     \
        if (field != NULL)                                                   \
        {                                                                    \
            return Device_field_get_ ## ftype(field);                        \
        }                                                                    \
        field = AAtree_get_exact(params->implement, (key));                  \
        if (field != NULL)                                                   \
        {                                                                    \
            return Device_field_get_ ## ftype(field);                        \
        }                                                                    \
    }                                                                        \
    else (void)0

bool* Device_params_get_bool(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonb"))
    {
        return NULL;
    }
    get_of_type(params, key, bool);
    return NULL;
}


int64_t* Device_params_get_int(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsoni"))
    {
        return NULL;
    }
    get_of_type(params, key, int);
    return NULL;
}


double* Device_params_get_float(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonf"))
    {
        return NULL;
    }
    get_of_type(params, key, float);
    return NULL;
}


Real* Device_params_get_real(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonr"))
    {
        return NULL;
    }
    get_of_type(params, key, real);
    return NULL;
}


Reltime* Device_params_get_reltime(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsont"))
    {
        return NULL;
    }
    get_of_type(params, key, reltime);
    return NULL;
}


Sample* Device_params_get_sample(Device_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".wv"))
    {
        return NULL;
    }
    get_of_type(params, key, sample);
    return NULL;
}


Sample_params* Device_params_get_sample_params(Device_params* params,
                                               const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonsh"))
    {
        return NULL;
    }
    get_of_type(params, key, sample_params);
    return NULL;
}


Sample_map* Device_params_get_sample_map(Device_params* params,
                                         const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (!string_has_suffix(key, ".jsonsm"))
    {
        return NULL;
    }
    get_of_type(params, key, sample_map);
    return NULL;
}

#undef get_of_type


void del_Device_params(Device_params* params)
{
    if (params == NULL)
    {
        return;
    }
    del_AAtree(params->implement);
    del_AAtree(params->config);
    del_AAtree(params->event_data);
    del_AAiter(params->event_data_iter);
    del_AAtree(params->slow_sync);
    del_AAiter(params->slow_sync_iter);
#if 0
    del_AAtree(params->event_names);
#endif
    xfree(params);
    return;
}


