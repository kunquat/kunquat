

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <ctype.h>
#include <string.h>

#include <devices/Device.h>
#include <devices/Device_field.h>
#include <devices/Device_impl.h>
#include <devices/Device_params.h>
#include <memory.h>
#include <string_common.h>
#include <Value.h>
#include <xassert.h>


typedef struct Set_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];

    union
    {
#define cb_info(type_name, def_val_type, param_type) \
        struct                                       \
        {                                            \
            def_val_type default_val;                \
            bool (*set)(                             \
                    Device_impl*,                    \
                    Device_key_indices,              \
                    param_type);                     \
            bool (*set_state)(                       \
                    const Device_impl*,              \
                    Device_state*,                   \
                    Device_key_indices,              \
                    param_type);                     \
        } type_name ## _type

        cb_info(bool, bool, bool);
        cb_info(float, double, double);
        cb_info(int, int64_t, int64_t);
        cb_info(Tstamp, Tstamp, const Tstamp*);
        cb_info(Envelope, const Envelope*, const Envelope*);
        cb_info(Sample, const Sample*, const Sample*);
        cb_info(Sample_params, const Sample_params*, const Sample_params*);
        cb_info(Note_map, const Note_map*, const Note_map*);
        cb_info(Hit_map, const Hit_map*, const Hit_map*);
        cb_info(Num_list, const Num_list*, const Num_list*);

#undef cb_info
    } cb;

} Set_cb;


typedef struct Update_state_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];
    Value_type type;

    union
    {
#define cb_info(type_name, param_type) \
        void (*update_ ## type_name)(  \
                const Device_impl*,    \
                Device_state*,         \
                Device_key_indices,    \
                param_type)

        cb_info(bool, bool);
        cb_info(float, double);
        cb_info(int, int64_t);
        cb_info(tstamp, const Tstamp*);

#undef cb_info
    } cb;

} Update_state_cb;


bool Device_impl_init(
        Device_impl* dimpl,
        void (*destroy)(Device_impl* dimpl))
{
    assert(dimpl != NULL);
    assert(destroy != NULL);

    dimpl->device = NULL;
    dimpl->set_cbs = NULL;
    dimpl->update_state_cbs = NULL;

    dimpl->set_audio_rate = NULL;
    dimpl->set_buffer_size = NULL;
    dimpl->update_tempo = NULL;
    dimpl->reset = NULL;
    dimpl->destroy = destroy;

    dimpl->set_cbs = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            memory_free);
    dimpl->update_state_cbs = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            memory_free);
    if (dimpl->set_cbs == NULL || dimpl->update_state_cbs == NULL)
    {
        Device_impl_deinit(dimpl);
        return false;
    }

    return true;
}


void Device_impl_register_set_audio_rate(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t))
{
    assert(dimpl != NULL);
    dimpl->set_audio_rate = set;
    return;
}


void Device_impl_register_set_buffer_size(
        Device_impl* dimpl,
        bool (*set)(const Device_impl*, Device_state*, int32_t))
{
    assert(dimpl != NULL);
    dimpl->set_buffer_size = set;
    return;
}


void Device_impl_register_update_tempo(
        Device_impl* dimpl,
        void (*update)(const Device_impl*, Device_state*, double))
{
    assert(dimpl != NULL);
    dimpl->update_tempo = update;
    return;
}


void Device_impl_register_reset_device_state(
        Device_impl* dimpl,
        void (*reset)(const Device_impl*, Device_state*))
{
    assert(dimpl != NULL);
    assert(reset != NULL);

    dimpl->reset = reset;

    return;
}


#define REGISTER_SET(type_name, type)                           \
    bool Device_impl_register_set_##type_name(                  \
            Device_impl* dimpl,                                 \
            const char* keyp,                                   \
            type default_val,                                   \
            bool (*set_func)(                                   \
                Device_impl*,                                   \
                Device_key_indices,                             \
                type),                                          \
            bool (*set_state_func)(                             \
                const Device_impl*,                             \
                Device_state*,                                  \
                Device_key_indices,                             \
                type))                                          \
    {                                                           \
        assert(dimpl != NULL);                                  \
        assert(keyp != NULL);                                   \
        assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);              \
        assert(set_func != NULL);                               \
                                                                \
        Set_cb* set_cb = memory_alloc_item(Set_cb);             \
        if (set_cb == NULL)                                     \
            return false;                                       \
                                                                \
        strcpy(set_cb->key_pattern, keyp);                      \
        set_cb->cb.type_name##_type.default_val = default_val;  \
        set_cb->cb.type_name##_type.set = set_func;             \
        set_cb->cb.type_name##_type.set_state = set_state_func; \
                                                                \
        if (!AAtree_ins(dimpl->set_cbs, set_cb))                \
        {                                                       \
            memory_free(set_cb);                                \
            return false;                                       \
        }                                                       \
                                                                \
        return true;                                            \
    }

REGISTER_SET(bool, bool)
REGISTER_SET(float, double)
REGISTER_SET(int, int64_t)

#undef REGISTER_SET

bool Device_impl_register_set_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        const Tstamp* default_val,
        bool (*set_func)(
            Device_impl*,
            Device_key_indices,
            const Tstamp*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Tstamp*))
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(default_val != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    Tstamp_copy(&set_cb->cb.Tstamp_type.default_val, default_val);
    set_cb->cb.Tstamp_type.set = set_func;
    set_cb->cb.Tstamp_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


bool Device_impl_register_set_envelope(
        Device_impl* dimpl,
        const char* keyp,
        const Envelope* default_val,
        bool (*set_func)(
            Device_impl*,
            Device_key_indices,
            const Envelope*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Envelope*))
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.Envelope_type.default_val = default_val;
    set_cb->cb.Envelope_type.set = set_func;
    set_cb->cb.Envelope_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


bool Device_impl_register_set_num_list(
        Device_impl* dimpl,
        const char* keyp,
        const Num_list* default_val,
        bool (*set_func)(
            Device_impl*,
            Device_key_indices,
            const Num_list*),
        bool (*set_state_func)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Num_list*))
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.Num_list_type.default_val = default_val;
    set_cb->cb.Num_list_type.set = set_func;
    set_cb->cb.Num_list_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


#define REGISTER_UPDATE(type_name, TYPE, ctype)                    \
    bool Device_impl_register_update_state_##type_name(            \
            Device_impl* dimpl,                                    \
            const char* keyp,                                      \
            void (*update_state)(                                  \
                const Device_impl*,                                \
                Device_state*,                                     \
                Device_key_indices,                                \
                ctype))                                            \
    {                                                              \
        assert(dimpl != NULL);                                     \
        assert(keyp != NULL);                                      \
        assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);                 \
        assert(update_state != NULL);                              \
                                                                   \
        Update_state_cb* update_state_cb = memory_alloc_item(      \
                Update_state_cb);                                  \
        if (update_state_cb == NULL)                               \
            return false;                                          \
                                                                   \
        strcpy(update_state_cb->key_pattern, keyp);                \
        update_state_cb->type = VALUE_TYPE_##TYPE;                 \
        update_state_cb->cb.update_##type_name = update_state;     \
                                                                   \
        if (!AAtree_ins(dimpl->update_state_cbs, update_state_cb)) \
        {                                                          \
            memory_free(update_state_cb);                          \
            return false;                                          \
        }                                                          \
                                                                   \
        return true;                                               \
    }

REGISTER_UPDATE(bool, BOOL, bool)
REGISTER_UPDATE(float, FLOAT, double)
REGISTER_UPDATE(int, INT, int64_t)

#undef REGISTER_UPDATE

bool Device_impl_register_update_state_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        void (*update_state)(
            const Device_impl*,
            Device_state*,
            Device_key_indices,
            const Tstamp*))
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(update_state != NULL);

    Update_state_cb* update_state_cb = memory_alloc_item(Update_state_cb);
    if (update_state_cb == NULL)
        return false;

    strcpy(update_state_cb->key_pattern, keyp);
    update_state_cb->type = VALUE_TYPE_TSTAMP;
    update_state_cb->cb.update_tstamp = update_state;

    if (!AAtree_ins(dimpl->update_state_cbs, update_state_cb))
    {
        memory_free(update_state_cb);
        return false;
    }

    return true;
}


void Device_impl_reset_device_state(
        const Device_impl* dimpl,
        Device_state* dstate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);

    if (dimpl->reset != NULL)
        dimpl->reset(dimpl, dstate);

    return;
}


bool Device_impl_set_audio_rate(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    if (dimpl->set_audio_rate != NULL)
        return dimpl->set_audio_rate(dimpl, dstate, audio_rate);

    return true;
}


bool Device_impl_set_buffer_size(
        const Device_impl* dimpl,
        Device_state* dstate,
        int32_t buffer_size)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(buffer_size >= 0);

    if (dimpl->set_buffer_size != NULL)
        return dimpl->set_buffer_size(dimpl, dstate, buffer_size);

    return true;
}


void Device_impl_update_tempo(
        const Device_impl* dimpl,
        Device_state* dstate,
        double tempo)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (dimpl->update_tempo != NULL)
        dimpl->update_tempo(dimpl, dstate, tempo);

    return;
}


static int32_t extract_num(
        const char* section,
        size_t section_length,
        size_t* digit_count)
{
    assert(section != NULL);
    assert(digit_count != NULL);

    int32_t num = 0;
    int mul = 1;

    *digit_count = 0;

    for (int pos = (int)section_length - 1; pos >= 0; --pos)
    {
        static const char* upper = "ABCDEF";
        if (!isxdigit(section[pos]) || strchr(upper, section[pos]) != NULL)
            break;

        static const char* hexdigits = "0123456789abcdef";
        assert(strchr(hexdigits, section[pos]) != NULL);

        num += (int32_t)(strchr(hexdigits, section[pos]) - hexdigits) * mul;

        mul <<= 4;
        ++(*digit_count);
    }

    if (*digit_count == 0)
        return -1;

    return num;
}


static void process_key(
        const char* key,
        char* keyp,
        Device_key_indices indices)
{
    assert(key != NULL);
    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    assert(keyp != NULL);
    assert(indices != NULL);

    int next_index_pos = 0;
    char* keyp_write_pos = keyp;

    const char* section = key;
    size_t section_length = strcspn(section, "/");

    while (section[section_length] != '\0')
    {
        // Check if there's a number at the end of the section
        size_t digit_count = 0;
        const int32_t num = extract_num(section, section_length, &digit_count);

        if (num >= 0)
        {
            // Store the number
            assert(next_index_pos < DEVICE_KEY_INDICES_MAX);
            indices[next_index_pos] = num;

            // Create a key pattern section of format "blabla_XXX/"
            assert(digit_count > 0);
            assert(digit_count <= section_length);
            const size_t prefix_length = section_length - digit_count;
            strncpy(keyp_write_pos, section, prefix_length);
            memset(keyp_write_pos + prefix_length, 'X', digit_count);
            keyp_write_pos[section_length] = '/';
        }
        else
        {
            // Copy the section as-is
            strncpy(keyp_write_pos, section, section_length);
            keyp_write_pos[section_length] = '/';
        }

        keyp_write_pos += section_length + 1;
        section = section + section_length + 1;
        section_length = strcspn(section, "/");
    }

    // Copy the last part of the key
    strncpy(keyp_write_pos, section, section_length);
    keyp_write_pos[section_length] = '\0';
    assert((int)strlen(key) == (keyp_write_pos + strlen(keyp_write_pos) - keyp));

    return;
}


bool Device_impl_set_key(Device_impl* dimpl, const char* key)
{
    assert(dimpl != NULL);
    assert(dimpl->device != NULL);
    assert(key != NULL);

    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    Device_key_indices indices = { 0 };
    memset(indices, '\xff', DEVICE_KEY_INDICES_MAX);

    process_key(key, keyp, indices);

    const Set_cb* set_cb = AAtree_get_exact(dimpl->set_cbs, keyp);
    if (set_cb != NULL)
    {
#define SET_FIELD(type_name, type)                                       \
        if (true)                                                        \
        {                                                                \
            const type* dval = Device_params_get_##type_name(            \
                    dimpl->device->dparams, key);                        \
            const type val = (dval != NULL) ?                            \
                *dval : set_cb->cb.type_name##_type.default_val;         \
            return set_cb->cb.type_name##_type.set(dimpl, indices, val); \
        }                                                                \
        else (void)0

#define SET_FIELDP(type_name, type)                                 \
        if (true)                                                   \
        {                                                           \
            const type* val = Device_params_get_##type_name(        \
                    dimpl->device->dparams, key);                   \
            return set_cb->cb.type##_type.set(dimpl, indices, val); \
        }                                                           \
        else (void)0

        const Device_field_type dftype = get_keyp_device_field_type(
                set_cb->key_pattern);
        assert(dftype != DEVICE_FIELD_NONE);

        switch (dftype)
        {
            case DEVICE_FIELD_BOOL:
                SET_FIELD(bool, bool);
                break;

            case DEVICE_FIELD_FLOAT:
                SET_FIELD(float, double);
                break;

            case DEVICE_FIELD_INT:
                SET_FIELD(int, int64_t);
                break;

            case DEVICE_FIELD_TSTAMP:
            {
                const Tstamp* dval = Device_params_get_tstamp(
                        dimpl->device->dparams, key);
                const Tstamp* val = (dval != NULL)
                    ? dval : &set_cb->cb.Tstamp_type.default_val;
                return set_cb->cb.Tstamp_type.set(dimpl, indices, val);
            }
            break;

            case DEVICE_FIELD_ENVELOPE:
                SET_FIELDP(envelope, Envelope);
                break;

            case DEVICE_FIELD_WAVPACK:
                SET_FIELDP(sample, Sample);
                break;

            case DEVICE_FIELD_SAMPLE_PARAMS:
                SET_FIELDP(sample_params, Sample_params);
                break;

            case DEVICE_FIELD_NOTE_MAP:
                SET_FIELDP(note_map, Note_map);
                break;

            case DEVICE_FIELD_HIT_MAP:
                SET_FIELDP(hit_map, Hit_map);
                break;

            case DEVICE_FIELD_NUM_LIST:
                SET_FIELDP(num_list, Num_list);
                break;

            default:
                assert(false);
        }

#undef SET_FIELDP
#undef SET_FIELD
    }

    return true;
}


bool Device_impl_set_state_key(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key)
{
    assert(dimpl != NULL);
    assert(key != NULL);
    assert(dstate != NULL);

    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    Device_key_indices indices = { 0 };
    memset(indices, '\xff', DEVICE_KEY_INDICES_MAX * sizeof(int32_t));

    process_key(key, keyp, indices);

    const Set_cb* set_cb = AAtree_get_exact(dimpl->set_cbs, keyp);
    if (set_cb != NULL)
    {
#define SET_FIELD(type_name, type)                               \
        if (true)                                                \
        {                                                        \
            const type* dval = Device_params_get_##type_name(    \
                    dimpl->device->dparams, key);                \
            const type val = (dval != NULL) ?                    \
                *dval : set_cb->cb.type_name##_type.default_val; \
            if (set_cb->cb.type_name##_type.set_state != NULL)   \
                return set_cb->cb.type_name##_type.set_state(    \
                        dimpl, dstate, indices, val);            \
        }                                                        \
        else (void)0

#define SET_FIELDP(type_name, type)                          \
        if (true)                                            \
        {                                                    \
            const type* val = Device_params_get_##type_name( \
                    dimpl->device->dparams, key);            \
            if (set_cb->cb.type##_type.set_state != NULL)    \
                return set_cb->cb.type##_type.set_state(     \
                        dimpl, dstate, indices, val);        \
        }                                                    \
        else (void)0

        const Device_field_type dftype = get_keyp_device_field_type(
                set_cb->key_pattern);
        assert(dftype != DEVICE_FIELD_NONE);

        switch (dftype)
        {
            case DEVICE_FIELD_BOOL:
                SET_FIELD(bool, bool);
                break;

            case DEVICE_FIELD_FLOAT:
                SET_FIELD(float, double);
                break;

            case DEVICE_FIELD_INT:
                SET_FIELD(int, int64_t);
                break;

            case DEVICE_FIELD_TSTAMP:
            {
                const Tstamp* dval = Device_params_get_tstamp(
                        dimpl->device->dparams, key);
                const Tstamp* val = (dval != NULL)
                    ? dval : &set_cb->cb.Tstamp_type.default_val;
                return set_cb->cb.Tstamp_type.set_state(
                        dimpl, dstate, indices, val);
            }
            break;

            case DEVICE_FIELD_ENVELOPE:
                SET_FIELDP(envelope, Envelope);
                break;

            case DEVICE_FIELD_WAVPACK:
                SET_FIELDP(sample, Sample);
                break;

            case DEVICE_FIELD_SAMPLE_PARAMS:
                SET_FIELDP(sample_params, Sample_params);
                break;

            case DEVICE_FIELD_NOTE_MAP:
                SET_FIELDP(note_map, Note_map);
                break;

            case DEVICE_FIELD_HIT_MAP:
                SET_FIELDP(hit_map, Hit_map);
                break;

            case DEVICE_FIELD_NUM_LIST:
                SET_FIELDP(num_list, Num_list);
                break;

            default:
                assert(false);
        }

#undef SET_FIELDP
#undef SET_FIELD
    }

    return true;
}


#define UPDATE_STATE_VALUE(type_name, type_upper, ctype)                   \
    void Device_impl_update_state_##type_name(                             \
            const Device_impl* dimpl,                                      \
            Device_state* dstate,                                          \
            const char* key,                                               \
            ctype value)                                                   \
    {                                                                      \
        assert(dimpl != NULL);                                             \
        assert(dstate != NULL);                                            \
        assert(key != NULL);                                               \
                                                                           \
        assert(strlen(key) < KQT_KEY_LENGTH_MAX);                          \
        char keyp[KQT_KEY_LENGTH_MAX] = "";                                \
        Device_key_indices indices = { 0 };                                \
        memset(indices, '\xff', DEVICE_KEY_INDICES_MAX * sizeof(int32_t)); \
                                                                           \
        process_key(key, keyp, indices);                                   \
                                                                           \
        const Update_state_cb* update_state_cb = AAtree_get_exact(         \
                dimpl->update_state_cbs,                                   \
                keyp);                                                     \
        if (update_state_cb != NULL &&                                     \
                update_state_cb->type == VALUE_TYPE_##type_upper)          \
            update_state_cb->cb.update_##type_name(                        \
                    dimpl,                                                 \
                    dstate,                                                \
                    indices,                                               \
                    value);                                                \
                                                                           \
        return;                                                            \
    }

UPDATE_STATE_VALUE(bool, BOOL, bool)
UPDATE_STATE_VALUE(float, FLOAT, double)
UPDATE_STATE_VALUE(int, INT, int64_t)

#undef UPDATE_VALUE


void Device_impl_update_state_tstamp(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(value != NULL);

    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    Device_key_indices indices = { 0 };
    memset(indices, '\xff', DEVICE_KEY_INDICES_MAX * sizeof(int32_t));

    process_key(key, keyp, indices);

    const Update_state_cb* update_state_cb = AAtree_get_exact(
            dimpl->update_state_cbs,
            keyp);
    if (update_state_cb != NULL && update_state_cb->type == VALUE_TYPE_TSTAMP)
        update_state_cb->cb.update_tstamp(
                dimpl,
                dstate,
                indices,
                value);

    return;
}


void Device_impl_deinit(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    del_AAtree(dimpl->update_state_cbs);
    dimpl->update_state_cbs = NULL;
    del_AAtree(dimpl->set_cbs);
    dimpl->set_cbs = NULL;

    return;
}


void del_Device_impl(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Device_impl_deinit(dimpl);

    assert(dimpl->destroy != NULL);
    dimpl->destroy(dimpl);

    return;
}


