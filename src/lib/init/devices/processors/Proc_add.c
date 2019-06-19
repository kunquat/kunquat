

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_add.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/Processor.h>
#include <init/devices/param_types/Sample.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <kunquat/limits.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/fft.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Add_state.h>
#include <string/common.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static float sine(double phase, double modifier);
static bool fill_buf(float* buf, const Sample* sample);

static Set_sample_func  Proc_add_set_base;
static Set_bool_func    Proc_add_set_ramp_attack;
static Set_bool_func    Proc_add_set_rand_phase;
static Set_float_func   Proc_add_set_tone_pitch;
static Set_float_func   Proc_add_set_tone_volume;
static Set_float_func   Proc_add_set_tone_panning;

static void del_Proc_add(Device_impl* dimpl);


Device_impl* new_Proc_add(void)
{
    Proc_add* add = memory_alloc_item(Proc_add);
    if (add == NULL)
        return NULL;

    add->base = NULL;
    add->is_ramp_attack_enabled = true;
    add->is_rand_phase_enabled = false;

    if (!Device_impl_init(&add->parent, del_Proc_add))
    {
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->parent.get_vstate_size = Add_vstate_get_size;
    add->parent.init_vstate = Add_vstate_init;
    add->parent.render_voice = Add_vstate_render_voice;

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(add, type, name, keyp, def_value)
#define REG_KEY_BOOL(name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(add, bool, name, keyp, def_value)

    if (!(REG_KEY(sample, base, "p_base.wav", NULL) &&
            REG_KEY_BOOL(ramp_attack, "p_b_ramp_attack.json", true) &&
            REG_KEY_BOOL(rand_phase, "p_b_rand_phase.json", false) &&
            REG_KEY(float, tone_pitch, "tone_XX/p_f_pitch.json", NAN) &&
            REG_KEY(float, tone_volume, "tone_XX/p_f_volume.json", NAN) &&
            REG_KEY(float, tone_panning, "tone_XX/p_f_pan.json", 0.0)
         ))
    {
        del_Device_impl(&add->parent);
        return NULL;
    }

#undef REG_KEY
#undef REG_KEY_BOOL

    // For easier resampling, * 4 adds space for a sample with double resolution
    // plus shorter waveforms progressively downsampled by a factor of 2
    float* buf = memory_alloc_items(float, ADD_BASE_FUNC_SIZE * 4);
    if (buf == NULL)
    {
        del_Device_impl(&add->parent);
        return NULL;
    }

    add->base = new_Sample_from_buffers(&buf, 1, ADD_BASE_FUNC_SIZE * 4);
    if (add->base == NULL)
    {
        memory_free(buf);
        del_Device_impl(&add->parent);
        return NULL;
    }

    fill_buf(Sample_get_buffer(add->base, 0), NULL);

    for (int h = 0; h < ADD_TONES_MAX; ++h)
    {
        add->tones[h].pitch_factor = 0;
        add->tones[h].volume_factor = 0;
        add->tones[h].panning = 0;
    }

    add->tones[0].pitch_factor = 1.0;
    add->tones[0].volume_factor = 1.0;

    return &add->parent;
}


static float sine(double phase, double modifier)
{
    ignore(modifier);
    return (float)(-sin(phase * PI * 2));
}


static bool fill_buf(float* buf, const Sample* sample)
{
    rassert(buf != NULL);

    rassert(is_p2(ADD_BASE_FUNC_SIZE));

    if ((sample != NULL) && (sample->data[0] != NULL) && sample->is_float)
    {
        FFT_worker* fw = FFT_worker_init(FFT_WORKER_AUTO, ADD_BASE_FUNC_SIZE * 2);
        if (fw == NULL)
            return false;

        for (int i = 0; i < ADD_BASE_FUNC_SIZE * 4; ++i)
            buf[i] = 0;

        // Get original sample data
        {
            const int available = (int)min(sample->len, ADD_BASE_FUNC_SIZE);

            const float* from_buf = sample->data[0];

            for (int i = 0; i < available; ++i)
                buf[i] = clamp(from_buf[i], -1.0f, 1.0f);
        }

        // Get frequency components
        FFT_worker_rfft(fw, buf, ADD_BASE_FUNC_SIZE);
        for (int i = 0; i < ADD_BASE_FUNC_SIZE; ++i)
            buf[i] *= 2;

        // Copy frequency components to smaller arrays
        {
            int from_offset = 0;
            int from_length = ADD_BASE_FUNC_SIZE * 2;
            int target_offset = from_length;

            for (; from_length > 1; from_length >>= 1)
            {
                const int copy_amount = (from_length / 2) - 1;
                for (int i = 0; i < copy_amount; ++i)
                    buf[target_offset + i] = buf[from_offset + i] / 2.0f;

                from_offset += from_length;
                target_offset += from_length / 2;
            }
        }

        // Get resampled waveforms
        {
            int start_offset = 0;
            for (int length = ADD_BASE_FUNC_SIZE * 2; length > 0; length >>= 1)
            {
                FFT_worker_irfft(fw, buf + start_offset, length);
                for (int i = 0; i < length; ++i)
                    buf[start_offset + i] /= (float)length;

                start_offset += length;
            }
        }

        FFT_worker_deinit(fw);
    }
    else
    {
        int start_offset = 0;
        for (int length = ADD_BASE_FUNC_SIZE * 2; length > 0; length >>= 1)
        {
            for (int i = 0; i < length; ++i)
                buf[start_offset + i] = sine((double)i / length, 0);

            start_offset += length;
        }
    }

    return true;
}


static bool Proc_add_set_base(
        Device_impl* dimpl, const Key_indices indices, const Sample* value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;

    return fill_buf(Sample_get_buffer(add->base, 0), value);
}


static bool Proc_add_set_ramp_attack(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->is_ramp_attack_enabled = enabled;

    return true;
}


static bool Proc_add_set_rand_phase(
        Device_impl* dimpl, const Key_indices indices, bool enabled)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    Proc_add* add = (Proc_add*)dimpl;
    add->is_rand_phase_enabled = enabled;

    return true;
}


static bool Proc_add_set_tone_pitch(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    const int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value > 0 && isfinite(value))
        add->tones[ti].pitch_factor = value;
    else
        add->tones[ti].pitch_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_volume(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    const int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (isfinite(value))
        add->tones[ti].volume_factor = dB_to_scale(value);
    else
        add->tones[ti].volume_factor = (ti == 0) ? 1.0 : 0.0;

    return true;
}


static bool Proc_add_set_tone_panning(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    rassert(indices != NULL);

    const int32_t ti = indices[0];
    if (ti < 0 || ti >= ADD_TONES_MAX)
        return true;

    Proc_add* add = (Proc_add*)dimpl;

    if (value >= -1.0 && value <= 1.0)
        add->tones[ti].panning = value;
    else
        add->tones[ti].panning = 0.0;

    return true;
}


static void del_Proc_add(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_add* add = (Proc_add*)dimpl;
    del_Sample(add->base);
    memory_free(add);

    return;
}


