

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
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

#include <debug/assert.h>
#include <devices/Device.h>
#include <devices/Device_field.h>
#include <devices/Device_impl.h>
#include <devices/Device_params.h>
#include <memory.h>
#include <string/common.h>
#include <string/key_pattern.h>
#include <Value.h>


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

#undef cb_info
    } cb;

} Set_cb;


typedef struct Update_control_var_cb
{
    char key_pattern[KQT_KEY_LENGTH_MAX];
    Value_type type;

    union
    {
        Set_cv_bool_func* set_cv_bool;

        Set_cv_int_func* set_cv_int;
        Set_cv_tstamp_func* set_cv_tstamp;

        struct
        {
            Set_cv_float_func* set_cv;
            Slide_target_cv_float_func* slide_target;
            Slide_length_cv_float_func* slide_length;
            Osc_speed_cv_float_func* osc_speed;
            Osc_depth_cv_float_func* osc_depth;
            Osc_speed_slide_length_cv_float_func* osc_speed_sl;
            Osc_depth_slide_length_cv_float_func* osc_depth_sl;
        } update_float;
    } cb;

} Update_control_var_cb;


void Device_impl_register_init(Device_impl* dimpl, bool (*init)(Device_impl*))
{
    assert(dimpl != NULL);
    assert(init != NULL);
    dimpl->init = init;
    return;
}


void Device_impl_register_destroy(Device_impl* dimpl, void (*destroy)(Device_impl*))
{
    assert(dimpl != NULL);
    assert(destroy != NULL);
    dimpl->destroy = destroy;
    return;
}


bool Device_impl_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);
    assert(dimpl->init != NULL);
    assert(dimpl->destroy != NULL);

    dimpl->set_cbs = NULL;
    dimpl->update_cv_cbs = NULL;

    dimpl->set_audio_rate = NULL;
    dimpl->set_buffer_size = NULL;
    dimpl->update_tempo = NULL;
    dimpl->reset = NULL;

    dimpl->set_cbs = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            memory_free);
    dimpl->update_cv_cbs = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            memory_free);
    if ((dimpl->set_cbs == NULL) ||
            (dimpl->update_cv_cbs == NULL) ||
            !dimpl->init(dimpl))
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


#define REGISTER_SET(type_name, type)                             \
    bool Device_impl_register_set_ ## type_name(                  \
            Device_impl* dimpl,                                   \
            const char* keyp,                                     \
            type default_val,                                     \
            Set_ ## type_name ## _func set_func,                  \
            Set_state_ ## type_name ## _func set_state_func)      \
    {                                                             \
        assert(dimpl != NULL);                                    \
        assert(keyp != NULL);                                     \
        assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);                \
        assert(set_func != NULL);                                 \
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
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(default_val != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

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
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

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
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

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
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_func != NULL);

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


#define REGISTER_SET_CV(type_name, TYPE, ctype)                    \
    bool Device_impl_register_set_cv_ ## type_name(                \
            Device_impl* dimpl,                                    \
            const char* keyp,                                      \
            Set_cv_ ## type_name ## _func set_cv)                  \
    {                                                              \
        assert(dimpl != NULL);                                     \
        assert(keyp != NULL);                                      \
        assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);                 \
        assert(set_cv != NULL);                                    \
                                                                   \
        Update_control_var_cb* update_cv_cb = memory_alloc_item(   \
                Update_control_var_cb);                            \
        if (update_cv_cb == NULL)                                  \
            return false;                                          \
                                                                   \
        strcpy(update_cv_cb->key_pattern, keyp);                   \
        update_cv_cb->type = VALUE_TYPE_##TYPE;                    \
        update_cv_cb->cb.set_cv_ ## type_name = set_cv;            \
                                                                   \
        if (!AAtree_ins(dimpl->update_cv_cbs, update_cv_cb))       \
        {                                                          \
            memory_free(update_cv_cb);                             \
            return false;                                          \
        }                                                          \
                                                                   \
        return true;                                               \
    }

REGISTER_SET_CV(bool, BOOL, bool)
REGISTER_SET_CV(int, INT, int64_t)

#undef REGISTER_UPDATE


bool Device_impl_register_updaters_cv_float(
        Device_impl* dimpl,
        const char* keyp,
        Set_cv_float_func set_cv,
        Slide_target_cv_float_func slide_target,
        Slide_length_cv_float_func slide_length,
        Osc_speed_cv_float_func osc_speed,
        Osc_depth_cv_float_func osc_depth,
        Osc_speed_slide_length_cv_float_func osc_speed_sl,
        Osc_depth_slide_length_cv_float_func osc_depth_sl)
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);

    Update_control_var_cb* update_cv_cb = memory_alloc_item(Update_control_var_cb);
    if (update_cv_cb == NULL)
        return false;

    strcpy(update_cv_cb->key_pattern, keyp);
    update_cv_cb->type = VALUE_TYPE_FLOAT;
    update_cv_cb->cb.update_float.set_cv = set_cv;
    update_cv_cb->cb.update_float.slide_target = slide_target;
    update_cv_cb->cb.update_float.slide_length = slide_length;
    update_cv_cb->cb.update_float.osc_speed = osc_speed;
    update_cv_cb->cb.update_float.osc_depth = osc_depth;
    update_cv_cb->cb.update_float.osc_speed_sl = osc_speed_sl;
    update_cv_cb->cb.update_float.osc_depth_sl = osc_depth_sl;

    if (!AAtree_ins(dimpl->update_cv_cbs, update_cv_cb))
    {
        memory_free(update_cv_cb);
        return false;
    }

    return true;
}


bool Device_impl_register_set_cv_tstamp(
        Device_impl* dimpl, const char* keyp, Set_cv_tstamp_func set_cv)
{
    assert(dimpl != NULL);
    assert(keyp != NULL);
    assert(strlen(keyp) < KQT_KEY_LENGTH_MAX);
    assert(set_cv != NULL);

    Update_control_var_cb* update_cv_cb = memory_alloc_item(Update_control_var_cb);
    if (update_cv_cb == NULL)
        return false;

    strcpy(update_cv_cb->key_pattern, keyp);
    update_cv_cb->type = VALUE_TYPE_TSTAMP;
    update_cv_cb->cb.set_cv_tstamp = set_cv;

    if (!AAtree_ins(dimpl->update_cv_cbs, update_cv_cb))
    {
        memory_free(update_cv_cb);
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
        const Device_impl* dimpl, Device_state* dstate, int32_t audio_rate)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(audio_rate > 0);

    if (dimpl->set_audio_rate != NULL)
        return dimpl->set_audio_rate(dimpl, dstate, audio_rate);

    return true;
}


bool Device_impl_set_buffer_size(
        const Device_impl* dimpl, Device_state* dstate, int32_t buffer_size)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(buffer_size >= 0);

    if (dimpl->set_buffer_size != NULL)
        return dimpl->set_buffer_size(dimpl, dstate, buffer_size);

    return true;
}


void Device_impl_update_tempo(
        const Device_impl* dimpl, Device_state* dstate, double tempo)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    if (dimpl->update_tempo != NULL)
        dimpl->update_tempo(dimpl, dstate, tempo);

    return;
}


bool Device_impl_set_key(Device_impl* dimpl, const char* key)
{
    assert(dimpl != NULL);
    assert(dimpl->device != NULL);
    assert(key != NULL);

    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
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
        else (void)0

#define SET_FIELDP(type_name, type)                                        \
        if (true)                                                          \
        {                                                                  \
            const type* val = Device_params_get_ ## type_name(             \
                    dimpl->device->dparams, key);                          \
            return set_cb->cb.type_name ## _type.set(dimpl, indices, val); \
        }                                                                  \
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
    Key_indices indices = { 0 };
    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    extract_key_pattern(key, keyp, indices);

    const Set_cb* set_cb = AAtree_get_exact(dimpl->set_cbs, keyp);
    if (set_cb != NULL)
    {
#define SET_FIELD(type_name, type)                                 \
        if (true)                                                  \
        {                                                          \
            const type* dval = Device_params_get_ ## type_name(    \
                    dimpl->device->dparams, key);                  \
            const type val = (dval != NULL) ?                      \
                *dval : set_cb->cb.type_name ## _type.default_val; \
            if (set_cb->cb.type_name ## _type.set_state != NULL)   \
                return set_cb->cb.type_name ## _type.set_state(    \
                        dimpl, dstate, indices, val);              \
        }                                                          \
        else (void)0

#define SET_FIELDP(type_name, type)                              \
        if (true)                                                \
        {                                                        \
            const type* val = Device_params_get_ ## type_name(   \
                    dimpl->device->dparams, key);                \
            if (set_cb->cb.type_name ## _type.set_state != NULL) \
                return set_cb->cb.type_name ## _type.set_state(  \
                        dimpl, dstate, indices, val);            \
        }                                                        \
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
                    ? dval : &set_cb->cb.tstamp_type.default_val;
                return set_cb->cb.tstamp_type.set_state(
                        dimpl, dstate, indices, val);
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

            default:
                assert(false);
        }

#undef SET_FIELDP
#undef SET_FIELD
    }

    return true;
}


static const Update_control_var_cb* get_update_control_var_cb(
        const Device_impl* dimpl, const char* key, Key_indices indices)
{
    assert(dimpl != NULL);
    assert(key != NULL);
    assert(indices != NULL);

    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    char keyp[KQT_KEY_LENGTH_MAX] = "";
    extract_key_pattern(key, keyp, indices);

    return AAtree_get_exact(dimpl->update_cv_cbs, keyp);
}


#define SET_CV(type_name, type_upper, ctype)                        \
    void Device_impl_set_cv_ ## type_name(                          \
            const Device_impl* dimpl,                               \
            Device_state* dstate,                                   \
            const char* key,                                        \
            ctype value)                                            \
    {                                                               \
        assert(dimpl != NULL);                                      \
        assert(dstate != NULL);                                     \
        assert(key != NULL);                                        \
        assert(strlen(key) < KQT_KEY_LENGTH_MAX);                   \
                                                                    \
        Key_indices indices = { 0 };                                \
        const Update_control_var_cb* update_cv_cb =                 \
            get_update_control_var_cb(dimpl, key, indices);         \
                                                                    \
        if (update_cv_cb != NULL &&                                 \
                update_cv_cb->type == VALUE_TYPE_ ## type_upper)    \
            update_cv_cb->cb.set_cv_ ## type_name(                  \
                    dimpl, dstate, indices, value);                 \
                                                                    \
        return;                                                     \
    }

SET_CV(bool, BOOL, bool)
SET_CV(int, INT, int64_t)

#undef SET_CV


void Device_impl_set_cv_float(
        const Device_impl* dimpl, Device_state* dstate, const char* key, double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    assert(isfinite(value));

    Key_indices indices = { 0 };
    const Update_control_var_cb* update_cv_cb =
        get_update_control_var_cb(dimpl, key, indices);

    if ((update_cv_cb != NULL) && (update_cv_cb->type == VALUE_TYPE_FLOAT))
        update_cv_cb->cb.update_float.set_cv(dimpl, dstate, indices, value);

    return;
}


void Device_impl_slide_cv_float_target(
        const Device_impl* dimpl, Device_state* dstate, const char* key, double value)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(strlen(key) < KQT_KEY_LENGTH_MAX);
    assert(isfinite(value));

    Key_indices indices = { 0 };
    const Update_control_var_cb* update_cv_cb =
        get_update_control_var_cb(dimpl, key, indices);

    if ((update_cv_cb != NULL) &&
            (update_cv_cb->type == VALUE_TYPE_FLOAT) &&
            (update_cv_cb->cb.update_float.slide_target != NULL))
        update_cv_cb->cb.update_float.slide_target(dimpl, dstate, indices, value);

    return;
}


void Device_impl_slide_cv_float_length(
        const Device_impl* dimpl,
        Device_state* dstate,
        const char* key,
        const Tstamp* length)
{
    assert(dimpl != NULL);
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Key_indices indices = { 0 };
    const Update_control_var_cb* update_cv_cb =
        get_update_control_var_cb(dimpl, key, indices);

    if ((update_cv_cb != NULL) &&
            (update_cv_cb->type == VALUE_TYPE_FLOAT) &&
            (update_cv_cb->cb.update_float.slide_length != NULL))
        update_cv_cb->cb.update_float.slide_length(dimpl, dstate, indices, length);

    return;
}


void Device_impl_set_cv_tstamp(
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
    Key_indices indices = { 0 };
    memset(indices, '\xff', KEY_INDICES_MAX * sizeof(int32_t));

    extract_key_pattern(key, keyp, indices);

    const Update_control_var_cb* update_cv_cb =
        AAtree_get_exact(dimpl->update_cv_cbs, keyp);
    if (update_cv_cb != NULL && update_cv_cb->type == VALUE_TYPE_TSTAMP)
        update_cv_cb->cb.set_cv_tstamp(dimpl, dstate, indices, value);

    return;
}


void Device_impl_deinit(Device_impl* dimpl)
{
    assert(dimpl != NULL);

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

    assert(dimpl->destroy != NULL);
    dimpl->destroy(dimpl);

    return;
}


