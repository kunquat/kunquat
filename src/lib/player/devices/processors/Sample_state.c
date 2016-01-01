

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


#include <player/devices/processors/Sample_state.h>

#include <debug/assert.h>
#include <init/devices/param_types/Sample.h>
#include <init/devices/param_types/Sample_params.h>
#include <init/devices/processors/Proc_sample.h>
#include <mathnum/common.h>
#include <player/Audio_buffer.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/Work_buffers.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct Sample_vstate
{
    Voice_state parent;
    int sample;
    double cents;
    double freq;
    double volume;
    uint8_t source;
    uint8_t expr;
    double middle_tone;
} Sample_vstate;


size_t Sample_vstate_get_size(void)
{
    return sizeof(Sample_vstate);
}


static int32_t Sample_render(
        const Sample* sample,
        const Sample_params* params,
        Voice_state* vstate,
        const Processor* proc,
        const Proc_state* proc_state,
        const Work_buffers* wbs,
        Audio_buffer* out_buffer,
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo,
        double middle_tone,
        double middle_freq,
        double vol_scale)
{
    assert(sample != NULL);
    assert(params != NULL);
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    assert(vol_scale >= 0);
    ignore(tempo);

    // This implementation does not support larger sample lengths :-P
    assert(sample->len < INT32_MAX - 1);

    if (sample->len == 0)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get actual pitches
    const Cond_work_buffer* actual_pitches = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_PITCHES),
            440,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_PITCH));

    // Get actual forces
    const Cond_work_buffer* actual_forces = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Work_buffers_get_buffer(wbs, WORK_BUFFER_ACTUAL_FORCES),
            1,
            Processor_is_voice_feature_enabled(proc, 0, VOICE_FEATURE_FORCE));

    float* abufs[KQT_BUFFERS_MAX] =
    {
        Audio_buffer_get_buffer(out_buffer, 0),
        Audio_buffer_get_buffer(out_buffer, 1),
    };

    static const int SAMPLE_WORK_BUFFER_POSITIONS = WORK_BUFFER_IMPL_1;
    static const int SAMPLE_WORK_BUFFER_NEXT_POSITIONS = WORK_BUFFER_IMPL_2;
    static const int SAMPLE_WORK_BUFFER_POSITIONS_REM = WORK_BUFFER_IMPL_3;

    int32_t* positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS);
    int32_t* next_positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_NEXT_POSITIONS);
    float* positions_rem = Work_buffers_get_buffer_contents_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS_REM);

    // Position information to be updated
    int32_t new_pos = vstate->pos;
    double new_pos_rem = vstate->pos_rem;

    // Get sample positions (assuming no loop at this point)
    const double shift_factor = middle_freq / (middle_tone * audio_rate);
    positions[buf_start] = new_pos;
    positions_rem[buf_start] = new_pos_rem;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float actual_pitch = Cond_work_buffer_get_value(actual_pitches, i);
        const double shift_total = actual_pitch * shift_factor;

        const int32_t shift_floor = floor(shift_total);
        const double shift_rem = shift_total - shift_floor;

        new_pos += shift_floor;
        new_pos_rem += shift_rem;
        const int32_t excess_whole = floor(new_pos_rem);
        new_pos += excess_whole;
        new_pos_rem -= excess_whole;

        positions[i + 1] = new_pos;
        positions_rem[i + 1] = new_pos_rem;
    }

    // Prevent invalid loop processing
    Sample_loop loop_mode = params->loop;
    if ((params->loop_end > sample->len) || (params->loop_start >= params->loop_end))
        loop_mode = SAMPLE_LOOP_OFF;

    // Apply loop and length constraints to sample positions
    int32_t new_buf_stop = buf_stop;
    switch (loop_mode)
    {
        case SAMPLE_LOOP_OFF:
        {
            const int32_t length = (int32_t)sample->len;

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                if (positions[i] >= length)
                {
                    new_buf_stop = i;
                    positions[i] = length - 1; // Make the index safe to access
                    vstate->active = false;
                    break;
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                next_positions[i] = positions[i] + 1;
            if (buf_start < new_buf_stop)
            {
                const int32_t last_index = new_buf_stop - 1;
                next_positions[last_index] = min(length - 1, next_positions[last_index]);
            }
        }
        break;

        case SAMPLE_LOOP_UNI:
        {
            const int32_t loop_start = params->loop_start;
            const int32_t loop_end = params->loop_end;
            const int32_t loop_length = loop_end - loop_start;

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t cur_pos = positions[i];
                if (cur_pos > loop_start)
                {
                    int32_t loop_pos = cur_pos - loop_start;
                    loop_pos %= loop_length;
                    cur_pos = loop_start + loop_pos;
                    positions[i] = cur_pos;
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t next_pos = positions[i] + 1;
                if (next_pos >= loop_end)
                    next_pos = loop_start;
                next_positions[i] = next_pos;
            }
        }
        break;

        case SAMPLE_LOOP_BI:
        {
            const int32_t loop_start = params->loop_start;
            const int32_t uni_loop_length = params->loop_end - loop_start - 1;
            const int32_t step_count = uni_loop_length * 2;
            const int32_t loop_length = max(1, step_count);

            // Current positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t cur_pos = positions[i];
                if (cur_pos > loop_start)
                {
                    int32_t loop_pos = cur_pos - loop_start;
                    loop_pos %= loop_length;
                    if (loop_pos >= uni_loop_length)
                        loop_pos = step_count - loop_pos;
                    cur_pos = loop_start + loop_pos;
                    positions[i] = cur_pos;
                    assert(cur_pos >= 0);
                }
            }

            // Next positions
            for (int32_t i = buf_start; i < buf_stop; ++i)
            {
                int32_t next_pos = positions[i] + 1;
                if (next_pos > loop_start)
                {
                    int32_t next_loop_pos = next_pos - loop_start;
                    next_loop_pos %= loop_length;
                    if (next_loop_pos >= uni_loop_length)
                        next_loop_pos = step_count - next_loop_pos;
                    next_pos = loop_start + next_loop_pos;
                }
                next_positions[i] = next_pos;
            }
        }
        break;

        default:
            assert(false);
    }

    // Get sample frames
#define get_item(out_value)                             \
    if (true)                                           \
    {                                                   \
        const int32_t cur_pos = positions[i];           \
        const int32_t next_pos = next_positions[i];     \
        const float lerp_value = positions_rem[i];      \
                                                        \
        const float cur_value = data[cur_pos];          \
        const float next_value = data[next_pos];        \
        const float diff = next_value - cur_value;      \
        (out_value) = cur_value + (lerp_value * diff);  \
    }                                                   \
    else (void)0

    if (!sample->is_float)
    {
        switch (sample->bits)
        {
            case 8:
            {
                static const double scale = 1.0 / 0x80;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int8_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = Cond_work_buffer_get_value(
                                actual_forces, i);
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            case 16:
            {
                static const double scale = 1.0 / 0x8000UL;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int16_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = Cond_work_buffer_get_value(
                                actual_forces, i);
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            case 32:
            {
                static const double scale = 1.0 / 0x80000000UL;
                const double fixed_scale = vol_scale * scale;
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int32_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float actual_force = Cond_work_buffer_get_value(
                                actual_forces, i);
                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * actual_force * fixed_scale;
                    }
                }
            }
            break;

            default:
                assert(false);
        }
    }
    else
    {
        for (int ch = 0; ch < sample->channels; ++ch)
        {
            const float* data = sample->data[ch];
            float* audio_buffer = abufs[ch];

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                const float actual_force = Cond_work_buffer_get_value(actual_forces, i);
                float item = 0;
                get_item(item);
                audio_buffer[i] = item * actual_force * vol_scale;
            }
        }
    }

#undef get_item

    // Copy mono signal to the right channel
    if (sample->channels == 1)
    {
        const int32_t frame_count = new_buf_stop - buf_start;
        memcpy(abufs[1] + buf_start, abufs[0] + buf_start, sizeof(float) * frame_count);
    }

    // Update position information
    vstate->pos = new_pos;
    vstate->pos_rem = new_pos_rem;

    return new_buf_stop;
}


static int32_t Sample_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

    Sample_vstate* sample_state = (Sample_vstate*)vstate;

    if (buf_start >= buf_stop)
        return buf_start;

    if (sample_state->sample < 0)
    {
        // Select our sample

        // TODO: remove the expression and source parameters from the format
        int expression = 0;
        int source = 0;

        const Sample_entry* entry = NULL;
        if (vstate->hit_index >= 0)
        {
            assert(vstate->hit_index < KQT_HITS_MAX);

            char map_key[] = "exp_X/src_X/p_hm_hit_map.json";
            snprintf(
                    map_key,
                    strlen(map_key) + 1,
                    "exp_%01x/src_%01x/p_hm_hit_map.json",
                    expression,
                    source);
            const Hit_map* map = Device_params_get_hit_map(
                    proc->parent.dparams,
                    map_key);
            if (map == NULL)
            {
                vstate->active = false;
                return buf_start;
            }

            vstate->pitch_controls.pitch = 440;
            entry = Hit_map_get_entry(
                    map,
                    vstate->hit_index,
                    vstate->force_controls.force,
                    vstate->rand_p);
        }
        else
        {
            char map_key[] = "exp_X/src_X/p_nm_note_map.json";
            snprintf(
                    map_key,
                    strlen(map_key) + 1,
                    "exp_%01x/src_%01x/p_nm_note_map.json",
                    expression,
                    source);
            const Note_map* map = Device_params_get_note_map(
                    proc->parent.dparams,
                    map_key);
            if (map == NULL)
            {
                vstate->active = false;
                return buf_start;
            }

            //fprintf(stderr, "pitch @ %p: %f\n", (void*)&state->pitch, state->pitch);
            entry = Note_map_get_entry(
                    map,
                    log2(vstate->pitch_controls.pitch / 440) * 1200,
                    vstate->force_controls.force,
                    vstate->rand_p);
            sample_state->middle_tone = entry->ref_freq;
        }

        if (entry == NULL || entry->sample >= SAMPLES_MAX)
        {
            vstate->active = false;
            return buf_start;
        }

        sample_state->sample = entry->sample;
        sample_state->volume = entry->vol_scale;
        sample_state->cents = entry->cents;
    }

    assert(sample_state->sample < SAMPLES_MAX);

    // Find sample params
    char header_key[] = "smp_XXX/p_sh_sample.json";
    snprintf(
            header_key,
            strlen(header_key) + 1,
            "smp_%03x/p_sh_sample.json",
            sample_state->sample);
    const Sample_params* header = Device_params_get_sample_params(
            proc->parent.dparams,
            header_key);
    if (header == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    assert(header->mid_freq > 0);
    assert(header->format > SAMPLE_FORMAT_NONE);

    // Find sample data
    static const char* extensions[] =
    {
        [SAMPLE_FORMAT_WAVPACK] = "wv",
    };

    char sample_key[] = "smp_XXX/p_sample.NONE";
    snprintf(sample_key, strlen(sample_key) + 1,
             "smp_%03x/p_sample.%s", sample_state->sample,
             extensions[header->format]);

    const Sample* sample = Device_params_get_sample(
            proc->parent.dparams, sample_key);
    if (sample == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    if (vstate->hit_index >= 0)
        sample_state->middle_tone = 440;

    sample_state->freq = header->mid_freq * exp2(sample_state->cents / 1200);

    /*
    Sample_set_loop_start(sample, sample_state->params.loop_start);
    Sample_set_loop_end(sample, sample_state->params.loop_end);
    Sample_set_loop(sample, sample_state->params.loop);
    // */

    Audio_buffer* out_buffer = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_SEND, 0);
    assert(out_buffer != NULL);

    const int32_t audio_rate = proc_state->parent.audio_rate;

    return Sample_render(
            sample, header, vstate, proc, proc_state, wbs,
            out_buffer, buf_start, buf_stop, audio_rate, tempo,
            sample_state->middle_tone, sample_state->freq,
            sample_state->volume);
}


void Sample_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Sample_vstate_render_voice;

    Sample_vstate* sample_state = (Sample_vstate*)vstate;
    sample_state->sample = -1;
    sample_state->cents = 0;
    sample_state->freq = 0;
    sample_state->volume = 0;
    sample_state->source = 0;
    sample_state->expr = 0;
    sample_state->middle_tone = 0;

    return;
}

