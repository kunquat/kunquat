

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
#include <devices/processors/Proc_sample.h>
#include <devices/processors/Proc_utils.h>

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

        int expression = 0;
        int source = 0;

        const int64_t* expression_arg = Channel_proc_state_get_int(
                vstate->cpstate, "e");
        if (expression_arg != NULL)
        {
            if (*expression_arg < 0 || *expression_arg >= SAMPLE_EXPRESSIONS_MAX)
            {
                vstate->active = false;
                return buf_start;
            }
            expression = *expression_arg;
        }

        const int64_t* source_arg = Channel_proc_state_get_int(
                vstate->cpstate, "s");
        if (source_arg != NULL)
        {
            if (*source_arg < 0 || *source_arg >= SAMPLE_SOURCES_MAX)
            {
                vstate->active = false;
                return buf_start;
            }
            source = *source_arg;
        }

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

    return Sample_process_vstate(
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


