

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
#include <math.h>
#include <stdio.h>

#include <AAtree.h>
#include <Generator_event_keys.h>
#include <Generator_field.h>
#include <Generator_params.h>
#include <string_common.h>

#include <xmemory.h>


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


struct Generator_params
{
    AAtree* implement;   ///< The implementation part of the generator.
    AAtree* config;      ///< The configuration part of the generator.
    AAtree* event_data;  ///< The playback state of the parameters.
    AAtree* event_names; ///< A mapping from event names to parameters.
};


bool key_is_generator_param(const char* key)
{
    assert(key != NULL);
    return key_is_real_time_generator_param(key) ||
           key_is_text_generator_param(key) ||
           string_has_suffix(key, ".wv") ||
           string_has_suffix(key, ".ogg");
}


bool key_is_real_time_generator_param(const char* key)
{
    assert(key != NULL);
    return string_has_suffix(key, ".jsonb") ||
           string_has_suffix(key, ".jsoni") ||
           string_has_suffix(key, ".jsonf") ||
           string_has_suffix(key, ".jsonr") ||
           string_has_suffix(key, ".jsont");
}


bool key_is_text_generator_param(const char* key)
{
    assert(key != NULL);
    return key_is_real_time_generator_param(key) ||
           string_has_suffix(key, ".jsone") ||
           string_has_suffix(key, ".jsonsm") ||
           string_has_suffix(key, ".jsonsh");
}


Generator_params* new_Generator_params(void)
{
    Generator_params* params = xalloc(Generator_params);
    if (params == NULL)
    {
        return NULL;
    }
    params->implement = NULL;
    params->config = NULL;
    params->event_data = NULL;
    params->implement = new_AAtree((int (*)(const void*,
                                            const void*))Generator_field_cmp,
                                   (void (*)(void*))del_Generator_field);
    params->config = new_AAtree((int (*)(const void*,
                                         const void*))Generator_field_cmp,
                                (void (*)(void*))del_Generator_field);
    params->event_data = new_AAtree((int (*)(const void*,
                                             const void*))Generator_field_cmp,
                                    (void (*)(void*))del_Generator_field);
    params->event_names = new_AAtree((int (*)(const void*, const void*))strcmp,
                                     free);
    if (params->implement == NULL || params->config == NULL ||
            params->event_data == NULL || params->event_names == NULL)
    {
        del_Generator_params(params);
        return NULL;
    }
    return params;
}


bool Generator_params_set_key(Generator_params* params, const char* key)
{
    assert(params != NULL);
    assert(key != NULL);
    if (AAtree_get_exact(params->event_data, key) != NULL)
    {
        return true;
    }
    Generator_field* field = new_Generator_field(key, NULL);
    if (field == NULL)
    {
        return false;
    }
    if (!AAtree_ins(params->event_data, field))
    {
        del_Generator_field(field);
        return false;
    }
    return true;
}


#define clean_if_fail()                      \
    if (true)                                \
    {                                        \
        if (state->error)                    \
        {                                    \
            del_AAtree(params->event_data);  \
            params->event_data = old_data;   \
            del_AAtree(params->event_names); \
            params->event_names = old_names; \
            return false;                    \
        }                                    \
    } else (void)0

bool Generator_params_parse_events(Generator_params* params,
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
                                             const void*))Generator_field_cmp,
                                    (void (*)(void*))del_Generator_field);
    if (params->event_data == NULL)
    {
        params->event_data = old_data;
        return false;
    }
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
    if (str == NULL)
    {
        del_AAtree(old_data);
        del_AAtree(old_names);
        return true;
    }
    str = read_const_char(str, '[', state);
    clean_if_fail();
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        del_AAtree(old_data);
        del_AAtree(old_names);
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
//        int64_t type = -1;
//        str = read_int(str, &type, state);
        bool channel_level = false;
        str = read_bool(str, &channel_level, state);
        str = read_const_char(str, ',', state);
        char param[100] = { '\0' };
        str = read_string(str, param, 99, state);
        clean_if_fail();

        if (!channel_level) // generator level
        {
            if (!key_is_real_time_generator_param(param))
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
            Generator_field* field = new_Generator_field(param, NULL);
            if (field == NULL || (/*Generator_field_set_event_control(field, true),*/
                                  !AAtree_ins(params->event_data, field)))
            {
                del_AAtree(params->event_names);
                params->event_names = old_names;
                del_AAtree(params->event_data);
                params->event_data = old_data;
                return false;
            }
        }
        else // if (type == 1) // channel level
        {
            if (!key_is_real_time_generator_param(param))
            {
                Read_state_set_error(state, "Key %s cannot be modified"
                                     " through events", param);
                clean_if_fail();
            }
            if (!Event_handler_add_channel_gen_state_key(eh, param))
            {
                del_AAtree(params->event_names);
                params->event_names = old_names;
                del_AAtree(params->event_data);
                params->event_data = old_data;
                return false;
            }
        }

        str = read_const_char(str, ']', state);
        clean_if_fail();
        check_next(str, state, expect_entry);
    }
    str = read_const_char(str, ']', state);
    clean_if_fail();
    
    del_AAtree(old_data);
    del_AAtree(old_names);
    return true;
}

#undef clean_if_fail


bool Generator_params_parse_value(Generator_params* params,
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
    assert(key_is_generator_param(key));
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
    Generator_field* field = AAtree_get_exact(tree, key);
    if (field != NULL)
    {
        return Generator_field_change(field, data, length, state);
    }
    else
    {
        field = new_Generator_field_from_data(key, data, length, state);
        if (field == NULL)
        {
            return false;
        }
        if (!AAtree_ins(tree, field))
        {
            del_Generator_field(field);
            return false;
        }
    }
    return true;
}


bool Generator_params_modify_value(Generator_params* params,
                                   const char* key,
                                   char* str)
{
    assert(params != NULL);
    assert(key != NULL);
    assert(key_is_real_time_generator_param(key));
    assert(str != NULL);
    Generator_field* field = AAtree_get_exact(params->event_data, key);
    if (field == NULL)
    {
        return false;
    }
    return Generator_field_modify(field, str);
}


#define get_of_type(params, key, ftype)                                         \
    if (true)                                                                   \
    {                                                                           \
        Generator_field* field = AAtree_get_exact((params)->event_data, (key)); \
        if (field != NULL && !Generator_field_get_empty(field))                 \
        {                                                                       \
            return Generator_field_get_ ## ftype(field);                        \
        }                                                                       \
        field = AAtree_get_exact(params->config, (key));                        \
        if (field != NULL)                                                      \
        {                                                                       \
            return Generator_field_get_ ## ftype(field);                        \
        }                                                                       \
        field = AAtree_get_exact(params->implement, (key));                     \
        if (field != NULL)                                                      \
        {                                                                       \
            return Generator_field_get_ ## ftype(field);                        \
        }                                                                       \
    }                                                                           \
    else (void)0

bool* Generator_params_get_bool(Generator_params* params, const char* key)
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


int64_t* Generator_params_get_int(Generator_params* params, const char* key)
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


double* Generator_params_get_float(Generator_params* params, const char* key)
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


Real* Generator_params_get_real(Generator_params* params, const char* key)
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


Reltime* Generator_params_get_reltime(Generator_params* params, const char* key)
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


Sample* Generator_params_get_sample(Generator_params* params, const char* key)
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


Sample_params* Generator_params_get_sample_params(Generator_params* params,
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


Sample_map* Generator_params_get_sample_map(Generator_params* params,
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


void del_Generator_params(Generator_params* params)
{
    assert(params != NULL);
    if (params->implement != NULL)
    {
        del_AAtree(params->implement);
    }
    if (params->config != NULL)
    {
        del_AAtree(params->config);
    }
    if (params->event_data != NULL)
    {
        del_AAtree(params->event_data);
    }
    if (params->event_names != NULL)
    {
        del_AAtree(params->event_names);
    }
    xfree(params);
    return;
}


