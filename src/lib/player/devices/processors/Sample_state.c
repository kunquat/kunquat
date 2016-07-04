

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
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
#include <mathnum/conversions.h>
#include <player/devices/processors/Proc_state_utils.h>
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
    double middle_tone;
} Sample_vstate;


int32_t Sample_vstate_get_size(void)
{
    return sizeof(Sample_vstate);
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_COUNT
};

enum
{
    PORT_OUT_AUDIO_L = 0,
    PORT_OUT_AUDIO_R,
    PORT_OUT_COUNT
};


static const int SAMPLE_WORK_BUFFER_POSITIONS = WORK_BUFFER_IMPL_1;
static const int SAMPLE_WORK_BUFFER_NEXT_POSITIONS = WORK_BUFFER_IMPL_2;
static const int SAMPLE_WORK_BUFFER_POSITIONS_REM = WORK_BUFFER_IMPL_3;
static const int SAMPLE_WB_FIXED_PITCH = WORK_BUFFER_IMPL_4;
static const int SAMPLE_WB_FIXED_FORCE = WORK_BUFFER_IMPL_5;


static int32_t Sample_render(
        const Sample* sample,
        const Sample_params* params,
        Voice_state* vstate,
        Proc_state* proc_state,
        const Work_buffers* wbs,
        float* out_buffers[2],
        int32_t buf_start,
        int32_t buf_stop,
        int32_t audio_rate,
        double tempo,
        double middle_tone,
        double middle_freq,
        double vol_scale)
{
    rassert(sample != NULL);
    rassert(params != NULL);
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(wbs != NULL);
    rassert(audio_rate > 0);
    rassert(tempo > 0);
    rassert(vol_scale >= 0);
    ignore(tempo);

    // This implementation does not support larger sample lengths :-P
    rassert(sample->len < INT32_MAX - 1);

    if (sample->len == 0)
    {
        vstate->active = false;
        return buf_start;
    }

    // Get frequencies
    Work_buffer* freqs_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
    const Work_buffer* pitches_wb = freqs_wb;
    if (freqs_wb == NULL)
        freqs_wb = Work_buffers_get_buffer_mut(wbs, SAMPLE_WB_FIXED_PITCH);
    Proc_fill_freq_buffer(freqs_wb, pitches_wb, buf_start, buf_stop);
    const float* freqs = Work_buffer_get_contents(freqs_wb);

    // Get force input
    Work_buffer* force_scales_wb = Proc_state_get_voice_buffer_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE);
    const Work_buffer* dBs_wb = force_scales_wb;
    if (force_scales_wb == NULL)
        force_scales_wb = Work_buffers_get_buffer_mut(wbs, SAMPLE_WB_FIXED_FORCE);
    Proc_fill_scale_buffer(force_scales_wb, dBs_wb, buf_start, buf_stop);
    const float* force_scales = Work_buffer_get_contents(force_scales_wb);

    float* abufs[KQT_BUFFERS_MAX] = { out_buffers[0], out_buffers[1] };
    if ((sample->channels == 1) && (out_buffers[0] == NULL))
    {
        // Make sure that mono sample is rendered to right channel
        // if the left one does not exist
        out_buffers[0] = out_buffers[1];
        out_buffers[1] = NULL;
    }

    int32_t* positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS);
    int32_t* next_positions = Work_buffers_get_buffer_contents_int_mut(
            wbs, SAMPLE_WORK_BUFFER_NEXT_POSITIONS);
    float* positions_rem = Work_buffers_get_buffer_contents_mut(
            wbs, SAMPLE_WORK_BUFFER_POSITIONS_REM);

    // Position information to be updated
    int32_t new_pos = (int32_t)vstate->pos;
    double new_pos_rem = vstate->pos_rem;

    // Get sample positions (assuming no loop at this point)
    const double shift_factor = middle_freq / (middle_tone * audio_rate);
    positions[buf_start] = new_pos;
    positions_rem[buf_start] = (float)new_pos_rem;

    for (int32_t i = buf_start; i < buf_stop; ++i)
    {
        const float freq = freqs[i];
        const double shift_total = freq * shift_factor;

        const int32_t shift_floor = (int32_t)floor(shift_total);
        const double shift_rem = shift_total - shift_floor;

        new_pos += shift_floor;
        new_pos_rem += shift_rem;
        const int32_t excess_whole = (int32_t)floor(new_pos_rem);
        new_pos += excess_whole;
        new_pos_rem -= excess_whole;

        positions[i + 1] = new_pos;
        positions_rem[i + 1] = (float)new_pos_rem;
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
                    Voice_state_set_finished(vstate);
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
            const int32_t loop_start = (int32_t)params->loop_start;
            const int32_t loop_end = (int32_t)params->loop_end;
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
            const int32_t loop_start = (int32_t)params->loop_start;
            const int32_t uni_loop_length = (int32_t)params->loop_end - loop_start - 1;
            const int32_t step_count = uni_loop_length * 2;
            const int32_t loop_length = max(1, step_count);

            // Next positions
            // NOTE: These must be processed first as the current positions
            //       will be wrapped in-place below
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
                    rassert(cur_pos >= 0);
                }
            }
        }
        break;

        default:
            rassert(false);
    }

    // Get sample frames
#define get_item(out_value)                             \
    if (true)                                           \
    {                                                   \
        const int32_t cur_pos = positions[i];           \
        const int32_t next_pos = next_positions[i];     \
        const float lerp_value = positions_rem[i];      \
                                                        \
        const float cur_value = (float)data[cur_pos];   \
        const float next_value = (float)data[next_pos]; \
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
                const float fixed_scale = (float)(vol_scale * scale);
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int8_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];
                    if (audio_buffer == NULL)
                        continue;

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float force_scale = force_scales[i];

                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * fixed_scale * force_scale;
                    }
                }
            }
            break;

            case 16:
            {
                static const double scale = 1.0 / 0x8000UL;
                const float fixed_scale = (float)(vol_scale * scale);
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int16_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];
                    if (audio_buffer == NULL)
                        continue;

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float force_scale = force_scales[i];

                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * fixed_scale * force_scale;
                    }
                }
            }
            break;

            case 32:
            {
                static const double scale = 1.0 / 0x80000000UL;
                const float fixed_scale = (float)(vol_scale * scale);
                for (int ch = 0; ch < sample->channels; ++ch)
                {
                    const int32_t* data = sample->data[ch];
                    float* audio_buffer = abufs[ch];
                    if (audio_buffer == NULL)
                        continue;

                    for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    {
                        const float force_scale = force_scales[i];

                        float item = 0;
                        get_item(item);
                        audio_buffer[i] = item * fixed_scale * force_scale;
                    }
                }
            }
            break;

            default:
                rassert(false);
        }
    }
    else
    {
        for (int ch = 0; ch < sample->channels; ++ch)
        {
            const float* data = sample->data[ch];
            float* audio_buffer = abufs[ch];
            if (audio_buffer == NULL)
                continue;

            for (int32_t i = buf_start; i < new_buf_stop; ++i)
            {
                const float force_scale = force_scales[i];

                float item = 0;
                get_item(item);
                audio_buffer[i] = (float)(item * vol_scale * force_scale);
            }
        }
    }

#undef get_item

    // Copy mono signal to the right channel
    if ((sample->channels == 1) && (abufs[0] != NULL) && (abufs[1] != NULL))
    {
        const int32_t frame_count = new_buf_stop - buf_start;
        rassert(frame_count >= 0);
        memcpy(abufs[1] + buf_start,
                abufs[0] + buf_start,
                sizeof(float) * (size_t)frame_count);
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
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(tempo > 0);

    const Processor* proc = (const Processor*)proc_state->parent.device;

    Sample_vstate* sample_state = (Sample_vstate*)vstate;

    if (buf_start >= buf_stop)
        return buf_start;

    // Get volume scales
    const Cond_work_buffer* vols = Cond_work_buffer_init(
            COND_WORK_BUFFER_AUTO,
            Proc_state_get_voice_buffer(
                proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_FORCE),
            0.0);

    if (sample_state->sample < 0)
    {
        // Select our sample

        const Sample_entry* entry = NULL;
        if (vstate->hit_index >= 0)
        {
            rassert(vstate->hit_index < KQT_HITS_MAX);

            const Hit_map* map =
                Device_params_get_hit_map(proc->parent.dparams, "p_hm_hit_map.json");
            if (map == NULL)
            {
                vstate->active = false;
                return buf_start;
            }

            entry = Hit_map_get_entry(
                    map,
                    vstate->hit_index,
                    Cond_work_buffer_get_value(vols, buf_start),
                    vstate->rand_p);
        }
        else
        {
            const Note_map* map =
                Device_params_get_note_map(proc->parent.dparams, "p_nm_note_map.json");
            if (map == NULL)
            {
                vstate->active = false;
                return buf_start;
            }

            //fprintf(stderr, "pitch @ %p: %f\n", (void*)&state->pitch, state->pitch);

            // Get starting pitch
            float start_pitch = 0;
            const float* pitches = Proc_state_get_voice_buffer_contents(
                    proc_state, DEVICE_PORT_TYPE_RECEIVE, PORT_IN_PITCH);
            if (pitches != NULL)
                start_pitch = pitches[buf_start];

            entry = Note_map_get_entry(
                    map,
                    start_pitch,
                    Cond_work_buffer_get_value(vols, buf_start),
                    vstate->rand_p);
            if (entry == NULL)
            {
                vstate->active = false;
                return buf_start;
            }

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

    rassert(sample_state->sample < SAMPLES_MAX);

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

    rassert(header->mid_freq > 0);
    rassert(header->format > SAMPLE_FORMAT_NONE);

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

    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(
            proc_state, PORT_OUT_AUDIO_L, PORT_OUT_COUNT, out_buffers);

    if ((out_buffers[0] == NULL) && (out_buffers[1] == NULL))
    {
        vstate->active = false;
        return buf_start;
    }

    const int32_t audio_rate = proc_state->parent.audio_rate;

    return Sample_render(
            sample, header, vstate, proc_state, wbs,
            out_buffers, buf_start, buf_stop, audio_rate, tempo,
            sample_state->middle_tone, sample_state->freq,
            sample_state->volume);
}


void Sample_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    vstate->render_voice = Sample_vstate_render_voice;

    Sample_vstate* sample_state = (Sample_vstate*)vstate;
    sample_state->sample = -1;
    sample_state->cents = 0;
    sample_state->freq = 0;
    sample_state->volume = 0;
    sample_state->middle_tone = 0;

    return;
}


