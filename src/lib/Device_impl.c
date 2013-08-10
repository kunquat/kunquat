

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
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

#include <Device_impl.h>
#include <memory.h>
#include <string_common.h>
#include <xassert.h>


typedef struct Update_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];

    union
    {
        struct
        {
            bool default_val;
            bool (*update)(
                    Device_impl*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    bool);
            bool (*update_state)(
                    const Device_impl*,
                    Device_state*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    bool);
        } bool_type;

        struct
        {
            double default_val;
            bool (*update)(
                    Device_impl*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    double);
            bool (*update_state)(
                    const Device_impl*,
                    Device_state*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    double);
        } float_type;

        struct
        {
            int64_t default_val;
            bool (*update)(
                    Device_impl*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    int64_t);
            bool (*update_state)(
                    const Device_impl*,
                    Device_state*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    int64_t);
        } int_type;

        struct
        {
            Tstamp default_val;
            bool (*update)(
                    Device_impl*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    const Tstamp*);
            bool (*update_state)(
                    const Device_impl*,
                    Device_state*,
                    int32_t[DEVICE_KEY_INDICES_MAX],
                    const Tstamp*);
        } Tstamp_type;
    } cb;

} Update_cb;


bool Device_impl_init(Device_impl* di, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(di != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    di->device = NULL;
    di->update_cbs = NULL;

    di->update_cbs = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            memory_free);
    if (di->update_cbs == NULL)
    {
        Device_impl_deinit(di);
        return false;
    }

    return true;
}


#define REGISTER_UPDATE(type_name, type)                            \
    bool Device_impl_register_update_##type_name(                   \
            Device_impl* di,                                        \
            const char* keyp,                                       \
            type default_val,                                       \
            bool (*update)(                                         \
                Device_impl*,                                       \
                int32_t[DEVICE_KEY_INDICES_MAX],                    \
                type),                                              \
            bool (*update_state)(                                   \
                const Device_impl*,                                 \
                Device_state*,                                      \
                int32_t[DEVICE_KEY_INDICES_MAX],                    \
                type))                                              \
    {                                                               \
        assert(di != NULL);                                         \
        assert(keyp != NULL);                                       \
        assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);                  \
        assert(update != NULL);                                     \
                                                                    \
        Update_cb* update_cb = memory_alloc_item(Update_cb);        \
        if (update_cb == NULL)                                      \
            return false;                                           \
                                                                    \
        strcpy(update_cb->key_pattern, keyp);                       \
        update_cb->cb.type_name##_type.default_val = default_val;   \
        update_cb->cb.type_name##_type.update = update;             \
        update_cb->cb.type_name##_type.update_state = update_state; \
                                                                    \
        if (!AAtree_ins(di->update_cbs, update_cb))                 \
        {                                                           \
            memory_free(update_cb);                                 \
            return false;                                           \
        }                                                           \
                                                                    \
        return true;                                                \
    }

REGISTER_UPDATE(bool, bool)
REGISTER_UPDATE(float, double)
REGISTER_UPDATE(int, int64_t)

#undef REGISTER_UPDATE

bool Device_impl_register_update_tstamp(
        Device_impl* di,
        const char* keyp,
        const Tstamp* default_val,
        bool (*update)(
            Device_impl*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*),
        bool (*update_state)(
            const Device_impl*,
            Device_state*,
            int32_t[DEVICE_KEY_INDICES_MAX],
            const Tstamp*))
{
    assert(di != NULL);
    assert(keyp != NULL);
    assert(default_val != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(update != NULL);

    Update_cb* update_cb = memory_alloc_item(Update_cb);
    if (update_cb == NULL)
        return false;

    strcpy(update_cb->key_pattern, keyp);
    Tstamp_copy(&update_cb->cb.Tstamp_type.default_val, default_val);
    update_cb->cb.Tstamp_type.update = update;
    update_cb->cb.Tstamp_type.update_state = update_state;

    if (!AAtree_ins(di->update_cbs, update_cb))
    {
        memory_free(update_cb);
        return false;
    }

    return true;
}


int32_t extract_num(
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
        int32_t indices[DEVICE_KEY_INDICES_MAX])
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
    assert(key + strlen(key) == keyp_write_pos + strlen(keyp_write_pos));

    return;
}


bool Device_impl_update_key(Device_impl* di, const char* key)
{
    assert(di != NULL);
    assert(key != NULL);

    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    int32_t indices[DEVICE_KEY_INDICES_MAX] = { 0 };

    process_key(key, keyp, indices);

    const Update_cb* update_cb = AAtree_get_exact(di->update_cbs, keyp);
    if (update_cb != NULL)
    {
        if (string_has_suffix(update_cb->key_pattern, ".jsonb"))
        {
        }
        else if (string_has_suffix(update_cb->key_pattern, ".jsonf"))
        {
        }
        else if (string_has_suffix(update_cb->key_pattern, ".jsoni"))
        {
        }
        else if (string_has_suffix(update_cb->key_pattern, ".jsont"))
        {
        }
    }

    return true;
}


void Device_impl_deinit(Device_impl* di)
{
    assert(di != NULL);

    del_AAtree(di->update_cbs);
    di->update_cbs = NULL;

    return;
}


void del_Device_impl(Device_impl* di)
{
    if (di == NULL)
        return;

    // TODO: callback!

    return;
}


