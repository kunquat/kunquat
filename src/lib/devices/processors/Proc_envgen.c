

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
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

#include <Audio_buffer.h>
#include <debug/assert.h>
#include <devices/Device_impl.h>
#include <devices/param_types/Envelope.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_envgen.h>
#include <devices/processors/Proc_utils.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/Proc_state.h>
#include <player/Time_env_state.h>
#include <string/common.h>


typedef struct Voice_state_envgen
{
    Voice_state parent;

    Time_env_state env_state;
} Voice_state_envgen;


typedef struct Proc_envgen
{
    Device_impl parent;

    double scale;

    bool is_time_env_enabled;
    const Envelope* time_env;
    bool is_loop_enabled;
    double env_scale_amount;
    double env_scale_center;

    bool is_force_env_enabled;
    const Envelope* force_env;

    double y_min;
    double y_max;
} Proc_envgen;


static bool Proc_envgen_init(Device_impl* dimpl);

static void Proc_envgen_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static Set_float_func       Proc_envgen_set_scale;
static Set_bool_func        Proc_envgen_set_time_env_enabled;
static Set_envelope_func    Proc_envgen_set_time_env;
static Set_bool_func        Proc_envgen_set_loop_enabled;
static Set_float_func       Proc_envgen_set_env_scale_amount;
static Set_float_func       Proc_envgen_set_env_scale_center;
static Set_bool_func        Proc_envgen_set_force_env_enabled;
static Set_envelope_func    Proc_envgen_set_force_env;
static Set_num_list_func    Proc_envgen_set_y_range;

static Proc_process_vstate_func Proc_envgen_process_vstate;

static void del_Proc_envgen(Device_impl* dimpl);


Device_impl* new_Proc_envgen(Processor* proc)
{
    Proc_envgen* egen = memory_alloc_item(Proc_envgen);
    if (egen == NULL)
        return NULL;

    egen->parent.device = (Device*)proc;

    Device_impl_register_init(&egen->parent, Proc_envgen_init);
    Device_impl_register_destroy(&egen->parent, del_Proc_envgen);

    egen->scale = 1;

    egen->is_time_env_enabled = false;
    egen->time_env = NULL;
    egen->is_loop_enabled = false;
    egen->env_scale_amount = 0;
    egen->env_scale_center = 440;

    egen->is_force_env_enabled = false;
    egen->force_env = NULL;

    egen->y_min = 0;
    egen->y_max = 1;

    return &egen->parent;
}


static bool Proc_envgen_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    Device_set_state_creator(dimpl->device, new_Proc_state_default);

    Processor* proc = (Processor*)egen->parent.device;
    proc->init_vstate = Proc_envgen_init_vstate;
    proc->process_vstate = Proc_envgen_process_vstate;

    bool reg_success = true;

#define REGISTER_SET(type, field, key, def_val)                         \
    reg_success &= Device_impl_register_set_##type(                     \
            &egen->parent, key, def_val, Proc_envgen_set_##field, NULL)

    REGISTER_SET(float,     scale,              "p_f_scale.json",               0.0);
    REGISTER_SET(bool,      time_env_enabled,   "p_b_time_env_enabled.json",    false);
    REGISTER_SET(envelope,  time_env,           "p_e_time_env.json",            NULL);
    REGISTER_SET(bool,      loop_enabled,       "p_b_loop_enabled.json",        false);
    REGISTER_SET(float,     env_scale_amount,   "p_f_env_scale_amount.json",    0.0);
    REGISTER_SET(float,     env_scale_center,   "p_f_env_scale_center.json",    0.0);
    REGISTER_SET(bool,      force_env_enabled,  "p_b_force_env_enabled.json",   false);
    REGISTER_SET(envelope,  force_env,          "p_e_force_env.json",           NULL);
    REGISTER_SET(num_list,  y_range,            "p_ln_y_range.json",            NULL);

#undef REGISTER_SET

    if (!reg_success)
        return false;

    return true;
}


const char* Proc_envgen_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    assert(property_type != NULL);

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = "";
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_envgen));

        return size_str;
    }

    return NULL;
}


static void Proc_envgen_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(vstate != NULL);

    Voice_state_envgen* egen_state = (Voice_state_envgen*)vstate;

    Time_env_state_init(&egen_state->env_state);

    return;
}


static uint32_t Proc_envgen_process_vstate(
        const Processor* proc,
        Proc_state* proc_state,
        Au_state* au_state,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo)
{
    assert(proc != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    Proc_envgen* egen = (Proc_envgen*)proc->parent.dimpl;
    Voice_state_envgen* egen_state = (Voice_state_envgen*)vstate;

    // Get output buffer for writing
    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);
    kqt_frame* audio_l = Audio_buffer_get_buffer(out_buffer, 0);
    kqt_frame* audio_r = Audio_buffer_get_buffer(out_buffer, 1);

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);
    const bool is_force_env_enabled =
        egen->is_force_env_enabled && (egen->force_env != NULL);

    const double range_width = egen->y_max - egen->y_min;

    int32_t new_buf_stop = buf_stop;

    // Initialise with default values
    for (int32_t i = buf_start; i < new_buf_stop; ++i)
        audio_l[i] = 1;

    if (is_time_env_enabled)
    {
        const int32_t env_stop = Time_env_state_process(
                &egen_state->env_state,
                egen->time_env,
                egen->is_loop_enabled,
                egen->env_scale_amount,
                egen->env_scale_center,
                0, // sustain
                0, 1, // range, NOTE: this needs to be mapped to our [y_min, y_max]!
                Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH),
                wbs,
                buf_start,
                new_buf_stop,
                audio_rate);

        float* time_env = Work_buffers_get_buffer_contents_mut(
                wbs, WORK_BUFFER_TIME_ENV);

        // Check the end of envelope processing
        if (egen_state->env_state.is_finished)
        {
            const double* last_node = Envelope_get_node(
                    egen->time_env, Envelope_node_count(egen->time_env) - 1);
            const double last_value = last_node[1];
            if (fabs(egen->y_min + last_value * range_width) < 0.0001)
            {
                vstate->active = false;
                new_buf_stop = env_stop;
            }
            else
            {
                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = env_stop; i < new_buf_stop; ++i)
                    time_env[i] = last_value;
            }
        }

        // Write to audio output
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] = time_env[i];
    }

    // Apply range
    if ((egen->y_min != 0) || (egen->y_max != 1))
    {
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] = egen->y_min + audio_l[i] * range_width;
    }

    // Apply our internal scaling
    if (egen->scale != 1)
    {
        const double scale = egen->scale;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            audio_l[i] *= scale;
    }

    if (Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE))
    {
        const float* actual_forces = Work_buffers_get_buffer_contents(
                wbs, WORK_BUFFER_ACTUAL_FORCES);

        if (is_force_env_enabled)
        {
            // Apply force envelope
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                const float actual_force = actual_forces[i];

                const double force_clamped = min(1, actual_force);
                const double factor = Envelope_get_value(egen->force_env, force_clamped);
                assert(isfinite(factor));
                audio_l[i] *= factor;
            }
        }
        else
        {
            // Apply linear scaling by default
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                audio_l[i] *= actual_forces[i];
        }
    }
    else
    {
        if (is_force_env_enabled)
        {
            // Just apply the rightmost force envelope value (as we assume force 1)
            const double factor = Envelope_get_node(
                    egen->force_env, Envelope_node_count(egen->force_env) - 1)[1];
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                audio_l[i] *= factor;
        }
    }

    // Copy to the right channel
    {
        const int32_t frame_count = new_buf_stop - buf_start;
        assert(frame_count >= 0);
        memcpy(audio_r + buf_start, audio_l + buf_start, sizeof(float) * frame_count);
    }

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return new_buf_stop;
}


static bool Proc_envgen_set_scale(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->scale = isfinite(value) ? exp2(value / 6) : 1;

    return true;
}


static bool Proc_envgen_set_time_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_time_env_enabled = value;

    return true;
}


static bool Proc_envgen_set_time_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = true;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        // Check the first node x coordinate
        {
            const double* node = Envelope_get_node(value, 0);
            if (node[0] != 0)
                is_valid = false;
        }

        // Check y coordinates
        for (int i = 0; i < Envelope_node_count(value); ++i)
        {
            const double* node = Envelope_get_node(value, i);
            if ((node[1] < 0) || (node[1] > 1))
            {
                is_valid = false;
                break;
            }
        }
    }
    else
    {
        is_valid = false;
    }

    egen->time_env = is_valid ? value : NULL;

    return true;
}


static bool Proc_envgen_set_loop_enabled(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_loop_enabled = value;

    return true;
}


static bool Proc_envgen_set_env_scale_amount(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->env_scale_amount = isfinite(value) ? value : 0;

    return true;
}


static bool Proc_envgen_set_env_scale_center(
        Device_impl* dimpl, Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->env_scale_center = isfinite(value) ? exp2(value / 1200) * 440 : 440;

    return true;
}


static bool Proc_envgen_set_force_env_enabled(
        Device_impl* dimpl, Key_indices indices, bool value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    egen->is_force_env_enabled = value;

    return true;
}


static bool Proc_envgen_set_force_env(
        Device_impl* dimpl, Key_indices indices, const Envelope* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = false;

    if ((value != NULL) &&
            (Envelope_node_count(value) > 1) &&
            (Envelope_node_count(value) <= 32))
    {
        // Check the endpoint x coordinates
        const double* first_node = Envelope_get_node(value, 0);
        const double* last_node = Envelope_get_node(
                value, Envelope_node_count(value) - 1);

        if ((first_node[0] == 0) && (last_node[0] == 1))
        {
            // Check y coordinates
            bool invalid_found = false;

            for (int i = 0; i < Envelope_node_count(value); ++i)
            {
                const double* node = Envelope_get_node(value, i);
                if ((node[1] < 0) || (node[1] > 1))
                {
                    invalid_found = true;
                    break;
                }
            }

            if (!invalid_found)
                is_valid = true;
        }
    }

    egen->force_env = is_valid ? value : NULL;

    return true;
}


static bool Proc_envgen_set_y_range(
        Device_impl* dimpl, Key_indices indices, const Num_list* value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_envgen* egen = (Proc_envgen*)dimpl;

    bool is_valid = false;

    if ((value != NULL) && (Num_list_length(value) == 2))
    {
        const double n1 = Num_list_get_num(value, 0);
        const double n2 = Num_list_get_num(value, 1);

        if (isfinite(n1) && isfinite(n2) && (n1 <= n2))
            is_valid = true;
    }

    if (is_valid)
    {
        egen->y_min = Num_list_get_num(value, 0);
        egen->y_max = Num_list_get_num(value, 1);
    }
    else
    {
        egen->y_min = 0;
        egen->y_max = 1;
    }

    return true;
}


static void del_Proc_envgen(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_envgen* egen = (Proc_envgen*)dimpl;
    memory_free(egen);

    return;
}


