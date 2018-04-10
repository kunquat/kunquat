

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Device_impl.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/Device_field.h>
#include <init/devices/Device_params.h>
#include <memory.h>
#include <string/common.h>
#include <string/key_pattern.h>
#include <Value.h>

#include <ctype.h>
#include <string.h>


typedef struct Set_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];

    union
    {
#define cb_info(type_name, def_val_type, param_type)     \
        struct                                           \
        {                                                \
            def_val_type default_val;                    \
            Set_ ## type_name ## _func* set;             \
            Set_state_ ## type_name ## _func* set_state; \
        } type_name ## _type

        cb_info(bool, bool, bool);
        cb_info(float, double, double);
        cb_info(int, int64_t, int64_t);
        cb_info(tstamp, Tstamp, const Tstamp*);
        cb_info(envelope, const Envelope*, const Envelope*);
        cb_info(sample, const Sample*, const Sample*);
        cb_info(sample_params, const Sample_params*, const Sample_params*);
        cb_info(note_map, const Note_map*, const Note_map*);
        cb_info(hit_map, const Hit_map*, const Hit_map*);
        cb_info(num_list, const Num_list*, const Num_list*);
        cb_info(padsynth_params, const Padsynth_params*, const Padsynth_params*);

#undef cb_info
    } cb;

} Set_cb;


typedef struct Device_impl_cv_bool_callbacks
{
    Proc_state_set_cv_bool_func* set_value;
    Voice_state_set_cv_bool_func* voice_set_value;
} Device_impl_cv_bool_callbacks;

typedef struct Device_impl_cv_int_callbacks
{
    Proc_state_set_cv_int_func* set_value;
    Voice_state_set_cv_int_func* voice_set_value;
} Device_impl_cv_int_callbacks;

typedef struct Device_impl_cv_float_callbacks
{
    Proc_state_set_cv_float_func* set_value;
    Voice_state_set_cv_float_func* voice_set_value;
} Device_impl_cv_float_callbacks;

typedef struct Device_impl_cv_tstamp_callbacks
{
    Proc_state_set_cv_tstamp_func* set_value;
    Voice_state_set_cv_tstamp_func* voice_set_value;
} Device_impl_cv_tstamp_callbacks;

typedef struct Update_control_var_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];
    Value_type type;

    union
    {
        Device_impl_cv_bool_callbacks bool_type;
        Device_impl_cv_int_callbacks int_type;
        Device_impl_cv_float_callbacks float_type;
        Device_impl_cv_tstamp_callbacks Tstamp_type;
    } cb;

} Update_control_var_cb;


bool Device_impl_init(Device_impl* dimpl, Device_impl_destroy_func* destroy)
{
    rassert(dimpl != NULL);
    rassert(destroy != NULL);

    dimpl->proc_type = Proc_type_COUNT;

    dimpl->create_pstate = NULL;
    dimpl->get_vstate_size = NULL;
    dimpl->get_voice_wb_size = NULL;
    dimpl->init_vstate = NULL;
    dimpl->render_voice = NULL;
    dimpl->fire_voice_dev_event = NULL;
    dimpl->destroy = destroy;

    dimpl->set_cbs = NULL;
    dimpl->update_cv_cbs = NULL;

    dimpl->set_cbs = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)memory_free);
    dimpl->update_cv_cbs = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)memory_free);
    if ((dimpl->set_cbs == NULL) || (dimpl->update_cv_cbs == NULL))
    {
        Device_impl_deinit(dimpl);
        return false;
    }

    return true;
}


void Device_impl_set_proc_type(Device_impl* dimpl, Proc_type proc_type)
{
    rassert(dimpl != NULL);
    rassert(proc_type >= 0);
    rassert(proc_type < Proc_type_COUNT);

    dimpl->proc_type = proc_type;

    return;
}


Proc_type Device_impl_get_proc_type(const Device_impl* dimpl)
{
    rassert(dimpl != NULL);
    return dimpl->proc_type;
}


void Device_impl_set_device(Device_impl* dimpl, const Device* device)
{
    rassert(dimpl != NULL);
    rassert(device != NULL);

    dimpl->device = device;

    return;
}


int32_t Device_impl_get_vstate_size(const Device_impl* dimpl)
{
    rassert(dimpl != NULL);

    if (dimpl->get_vstate_size != NULL)
        return dimpl->get_vstate_size();

    return sizeof(Voice_state);
}


int32_t Device_impl_get_voice_wb_size(const Device_impl* dimpl, int32_t audio_rate)
{
    rassert(dimpl != NULL);
    rassert(audio_rate > 0);

    if (dimpl->get_voice_wb_size != NULL)
        return dimpl->get_voice_wb_size(dimpl, audio_rate);

    return 0;
}


#define REGISTER_SET(type_name, type)                             \
    bool Device_impl_register_set_ ## type_name(                  \
            Device_impl* dimpl,                                   \
            const char* keyp,                                     \
            type default_val,                                     \
            Set_ ## type_name ## _func set_func,                  \
            Set_state_ ## type_name ## _func set_state_func)      \
    {                                                             \
        rassert(dimpl != NULL);                                   \
        rassert(keyp != NULL);                                    \
        rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);               \
        rassert(set_func != NULL);                                \
                                                                  \
        Set_cb* set_cb = memory_alloc_item(Set_cb);               \
        if (set_cb == NULL)                                       \
            return false;                                         \
                                                                  \
        strcpy(set_cb->key_pattern, keyp);                        \
        set_cb->cb.type_name ## _type.default_val = default_val;  \
        set_cb->cb.type_name ## _type.set = set_func;             \
        set_cb->cb.type_name ## _type.set_state = set_state_func; \
                                                                  \
        if (!AAtree_ins(dimpl->set_cbs, set_cb))                  \
        {                                                         \
            memory_free(set_cb);                                  \
            return false;                                         \
        }                                                         \
                                                                  \
        return true;                                              \
    }

REGISTER_SET(bool, bool)
REGISTER_SET(float, double)
REGISTER_SET(int, int64_t)

#undef REGISTER_SET

bool Device_impl_register_set_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        const Tstamp* default_val,
        Set_tstamp_func set_func,
        Set_state_tstamp_func set_state_func)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);
    rassert(default_val != NULL);
    rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    rassert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    Tstamp_copy(&set_cb->cb.tstamp_type.default_val, default_val);
    set_cb->cb.tstamp_type.set = set_func;
    set_cb->cb.tstamp_type.set_state = set_state_func;

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
        Set_envelope_func set_func,
        Set_state_envelope_func set_state_func)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);
    rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    rassert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.envelope_type.default_val = default_val;
    set_cb->cb.envelope_type.set = set_func;
    set_cb->cb.envelope_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


bool Device_impl_register_set_sample(
        Device_impl* dimpl,
        const char* keyp,
        const Sample* default_val,
        Set_sample_func set_func,
        Set_state_sample_func set_state_func)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);
    rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    rassert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.sample_type.default_val = default_val;
    set_cb->cb.sample_type.set = set_func;
    set_cb->cb.sample_type.set_state = set_state_func;

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
        Set_num_list_func set_func,
        Set_state_num_list_func set_state_func)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);
    rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    rassert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.num_list_type.default_val = default_val;
    set_cb->cb.num_list_type.set = set_func;
    set_cb->cb.num_list_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


bool Device_impl_register_set_padsynth_params(
        Device_impl* dimpl,
        const char* keyp,
        const Padsynth_params* default_val,
        Set_padsynth_params_func set_func,
        Set_state_padsynth_params_func set_state_func)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);
    rassert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    rassert(set_func != NULL);

    Set_cb* set_cb = memory_alloc_item(Set_cb);
    if (set_cb == NULL)
        return false;

    strcpy(set_cb->key_pattern, keyp);
    set_cb->cb.padsynth_params_type.default_val = default_val;
    set_cb->cb.padsynth_params_type.set = set_func;
    set_cb->cb.padsynth_params_type.set_state = set_state_func;

    if (!AAtree_ins(dimpl->set_cbs, set_cb))
    {
        memory_free(set_cb);
        return false;
    }

    return true;
}


static Update_control_var_cb* Device_impl_create_update_cv_cb(
        Device_impl* dimpl, const char* keyp, Value_type type)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);

    Update_control_var_cb* update_cv_cb = memory_alloc_item(
            Update_control_var_cb);
    if (update_cv_cb == NULL)
        return NULL;

    strcpy(update_cv_cb->key_pattern, keyp);
    update_cv_cb->type = type;

    if (!AAtree_ins(dimpl->update_cv_cbs, update_cv_cb))
    {
        memory_free(update_cv_cb);
        return NULL;
    }

    return update_cv_cb;
}


bool Device_impl_create_cv_bool(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_bool_func* pstate_set,
        Voice_state_set_cv_bool_func* vstate_set)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);

    Update_control_var_cb* update_cv_cb =
        Device_impl_create_update_cv_cb(dimpl, keyp, VALUE_TYPE_BOOL);
    if (update_cv_cb == NULL)
        return false;

    update_cv_cb->cb.bool_type.set_value = pstate_set;
    update_cv_cb->cb.bool_type.voice_set_value = vstate_set;

    return true;
}


bool Device_impl_create_cv_int(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_int_func* pstate_set,
        Voice_state_set_cv_int_func* vstate_set)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);

    Update_control_var_cb* update_cv_cb =
        Device_impl_create_update_cv_cb(dimpl, keyp, VALUE_TYPE_INT);
    if (update_cv_cb == NULL)
        return false;

    update_cv_cb->cb.int_type.set_value = pstate_set;
    update_cv_cb->cb.int_type.voice_set_value = vstate_set;

    return true;
}


bool Device_impl_create_cv_float(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_float_func* pstate_set,
        Voice_state_set_cv_float_func* vstate_set)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);

    Update_control_var_cb* update_cv_cb =
        Device_impl_create_update_cv_cb(dimpl, keyp, VALUE_TYPE_FLOAT);
    if (update_cv_cb == NULL)
        return false;

    update_cv_cb->cb.float_type.set_value = pstate_set;
    update_cv_cb->cb.float_type.voice_set_value = vstate_set;

    return true;
}


bool Device_impl_create_cv_tstamp(
        Device_impl* dimpl,
        const char* keyp,
        Proc_state_set_cv_tstamp_func* pstate_set,
        Voice_state_set_cv_tstamp_func* vstate_set)
{
    rassert(dimpl != NULL);
    rassert(keyp != NULL);

    Update_control_var_cb* update_cv_cb =
        Device_impl_create_update_cv_cb(dimpl, keyp, VALUE_TYPE_TSTAMP);
    if (update_cv_cb == NULL)
        return false;

    update_cv_cb->cb.Tstamp_type.set_value = pstate_set;
    update_cv_cb->cb.Tstamp_type.voice_set_value = vstate_set;

    return true;
}


bool Device_impl_set_key(Device_impl* dimpl, const char* key)
{
    rassert(dimpl != NULL);
    rassert(dimpl->device != NULL);
    rassert(key != NULL);

    rassert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    Key_indices indices = { 0 };
    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    extract_key_pattern(key, keyp, indices);

    const Set_cb* set_cb = AAtree_get_exact(dimpl->set_cbs, keyp);
    if (set_cb != NULL)
    {
#define SET_FIELD(type_name, type)                                         \
        if (true)                                                          \
        {                                                                  \
            const type* dval = Device_params_get_ ## type_name(            \
                    dimpl->device->dparams, key);                          \
            const type val = (dval != NULL) ?                              \
                *dval : set_cb->cb.type_name ## _type.default_val;         \
            return set_cb->cb.type_name ## _type.set(dimpl, indices, val); \
        }                                                                  \
        else ignore(0)

#define SET_FIELDP(type_name, type)                                        \
        if (true)                                                          \
        {                                                                  \
            const type* val = Device_params_get_ ## type_name(             \
                    dimpl->device->dparams, key);                          \
            return set_cb->cb.type_name ## _type.set(dimpl, indices, val); \
        }                                                                  \
        else ignore(0)

        const Device_field_type dftype = get_keyp_device_field_type(
                set_cb->key_pattern);
        rassert(dftype != DEVICE_FIELD_NONE);

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
                    ? dval : &set_cb->cb.tstamp_type.default_val;
                return set_cb->cb.tstamp_type.set(dimpl, indices, val);
            }
            break;

            case DEVICE_FIELD_ENVELOPE:
                SET_FIELDP(envelope, Envelope);
                break;

            case DEVICE_FIELD_WAVPACK:
            case DEVICE_FIELD_WAV:
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

            case DEVICE_FIELD_PADSYNTH_PARAMS:
                SET_FIELDP(padsynth_params, Padsynth_params);
                break;

            default:
                rassert(false);
        }

#undef SET_FIELDP
#undef SET_FIELD
    }

    return true;
}


bool Device_impl_set_state_key(
        const Device_impl* dimpl, Device_state* dstate, const char* key)
{
    rassert(dimpl != NULL);
    rassert(key != NULL);
    rassert(dstate != NULL);

    rassert(strlen(key) < KQT_KEY_LENGTH_MAX);
    char keyp[KQT_KEY_LENGTH_MAX] = "";
    Key_indices indices = { 0 };
    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    extract_key_pattern(key, keyp, indices);

    const Set_cb* set_cb = AAtree_get_exact(dimpl->set_cbs, keyp);
    if (set_cb != NULL)
    {
#define SET_FIELD(type_name, type)                                                    \
        if (true)                                                                     \
        {                                                                             \
            const type* dval = Device_params_get_ ## type_name(                       \
                    dimpl->device->dparams, key);                                     \
            const type val = (dval != NULL) ?                                         \
                *dval : set_cb->cb.type_name ## _type.default_val;                    \
            if (set_cb->cb.type_name ## _type.set_state != NULL)                      \
                return set_cb->cb.type_name ## _type.set_state(dstate, indices, val); \
        }                                                                             \
        else ignore(0)

#define SET_FIELDP(type_name, type)                                                   \
        if (true)                                                                     \
        {                                                                             \
            const type* val = Device_params_get_ ## type_name(                        \
                    dimpl->device->dparams, key);                                     \
            if (set_cb->cb.type_name ## _type.set_state != NULL)                      \
                return set_cb->cb.type_name ## _type.set_state(dstate, indices, val); \
        }                                                                             \
        else ignore(0)

        const Device_field_type dftype = get_keyp_device_field_type(set_cb->key_pattern);
        rassert(dftype != DEVICE_FIELD_NONE);

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
                    ? dval : &set_cb->cb.tstamp_type.default_val;
                return set_cb->cb.tstamp_type.set_state(dstate, indices, val);
            }
            break;

            case DEVICE_FIELD_ENVELOPE:
                SET_FIELDP(envelope, Envelope);
                break;

            case DEVICE_FIELD_WAVPACK:
            case DEVICE_FIELD_WAV:
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

            case DEVICE_FIELD_PADSYNTH_PARAMS:
                SET_FIELDP(padsynth_params, Padsynth_params);
                break;

            default:
                rassert(false);
        }

#undef SET_FIELDP
#undef SET_FIELD
    }

    return true;
}


static const Update_control_var_cb* get_update_control_var_cb(
        const Device_impl* dimpl, const char* key, Key_indices indices)
{
    rassert(dimpl != NULL);
    rassert(key != NULL);
    rassert(indices != NULL);

    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    char keyp[KQT_KEY_LENGTH_MAX] = "";
    extract_key_pattern(key, keyp, indices);

    return AAtree_get_exact(dimpl->update_cv_cbs, keyp);
}


void Device_impl_get_proc_cv_callback(
        const Device_impl* dimpl,
        const char* key,
        Value_type type,
        Device_impl_proc_cv_callback* cb)
{
    rassert(dimpl != NULL);
    rassert(key != NULL);
    rassert(cb != NULL);

    const Update_control_var_cb* update_cv_cb =
        get_update_control_var_cb(dimpl, key, cb->indices);

    if ((update_cv_cb == NULL) || (type != update_cv_cb->type))
    {
        cb->type = VALUE_TYPE_NONE;
        return;
    }

    cb->type = type;

    switch (type)
    {
        case VALUE_TYPE_BOOL:
            cb->cb.set_bool = update_cv_cb->cb.bool_type.set_value;
            break;

        case VALUE_TYPE_INT:
            cb->cb.set_int = update_cv_cb->cb.int_type.set_value;
            break;

        case VALUE_TYPE_FLOAT:
            cb->cb.set_float = update_cv_cb->cb.float_type.set_value;
            break;

        case VALUE_TYPE_TSTAMP:
            cb->cb.set_tstamp = update_cv_cb->cb.Tstamp_type.set_value;
            break;

        default:
            rassert(false);
    }

    return;
}


void Device_impl_get_voice_cv_callback(
        const Device_impl* dimpl,
        const char* key,
        Value_type type,
        Device_impl_voice_cv_callback* cb)
{
    rassert(dimpl != NULL);
    rassert(key != NULL);
    rassert(cb != NULL);

    const Update_control_var_cb* update_cv_cb =
        get_update_control_var_cb(dimpl, key, cb->indices);

    if ((update_cv_cb == NULL) || (type != update_cv_cb->type))
    {
        cb->type = VALUE_TYPE_NONE;
        return;
    }

    cb->type = type;

    switch (type)
    {
        case VALUE_TYPE_BOOL:
            cb->cb.set_bool = update_cv_cb->cb.bool_type.voice_set_value;
            break;

        case VALUE_TYPE_INT:
            cb->cb.set_int = update_cv_cb->cb.int_type.voice_set_value;
            break;

        case VALUE_TYPE_FLOAT:
            cb->cb.set_float = update_cv_cb->cb.float_type.voice_set_value;
            break;

        case VALUE_TYPE_TSTAMP:
            cb->cb.set_tstamp = update_cv_cb->cb.Tstamp_type.voice_set_value;
            break;

        default:
            rassert(false);
    }

    return;
}


void Device_impl_deinit(Device_impl* dimpl)
{
    rassert(dimpl != NULL);

    del_AAtree(dimpl->update_cv_cbs);
    dimpl->update_cv_cbs = NULL;
    del_AAtree(dimpl->set_cbs);
    dimpl->set_cbs = NULL;

    return;
}


void del_Device_impl(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Device_impl_deinit(dimpl);

    rassert(dimpl->destroy != NULL);
    dimpl->destroy(dimpl);

    return;
}


