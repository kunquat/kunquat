

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <math.h>
#include <stdio.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <devices/Device_params.h>
#include <devices/Processor.h>
#include <devices/processors/Proc_pcm.h>
#include <devices/processors/Voice_state_pcm.h>
#include <devices/param_types/Hit_map.h>
#include <devices/param_types/Sample.h>
#include <devices/param_types/Wavpack.h>
#include <memory.h>
#include <pitch_t.h>
#include <player/Work_buffers.h>
#include <string/common.h>


typedef struct Proc_pcm
{
    Device_impl parent;
} Proc_pcm;


static bool Proc_pcm_init(Device_impl* dimpl);

static void Proc_pcm_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate);

static Proc_process_vstate_func Proc_pcm_process_vstate;

static void del_Proc_pcm(Device_impl* dimpl);


Device_impl* new_Proc_pcm(Processor* proc)
{
    Proc_pcm* pcm = memory_alloc_item(Proc_pcm);
    if (pcm == NULL)
        return NULL;

    pcm->parent.device = (Device*)proc;

    Device_impl_register_init(&pcm->parent, Proc_pcm_init);
    Device_impl_register_destroy(&pcm->parent, del_Proc_pcm);

    return &pcm->parent;
}


static bool Proc_pcm_init(Device_impl* dimpl)
{
    assert(dimpl != NULL);

    Proc_pcm* pcm = (Proc_pcm*)dimpl;

    Processor* proc = (Processor*)pcm->parent.device;
    proc->init_vstate = Proc_pcm_init_vstate;
    proc->process_vstate = Proc_pcm_process_vstate;

    return true;
}


const char* Proc_pcm_property(const Processor* proc, const char* property_type)
{
    assert(proc != NULL);
    //assert(string_eq(proc->type, "pcm"));
    assert(property_type != NULL);
    (void)proc;

    if (string_eq(property_type, "voice_state_size"))
    {
        static char size_str[8] = { '\0' };
        if (string_eq(size_str, ""))
            snprintf(size_str, 8, "%zd", sizeof(Voice_state_pcm));

        return size_str;
    }
    else if (string_eq(property_type, "proc_state_vars"))
    {
        static const char* vars_str = "["
            "[\"I\", \"e\"], " // expression
            "[\"I\", \"s\"]"   // source
            "]";
        return vars_str;
    }

    return NULL;
}


static void Proc_pcm_init_vstate(
        const Processor* proc, const Proc_state* proc_state, Voice_state* vstate)
{
    assert(proc != NULL);
    //assert(string_eq(proc->type, "pcm"));
    (void)proc;
    assert(proc_state != NULL);
    (void)proc_state;
    assert(vstate != NULL);

    Voice_state_pcm* pcm_state = (Voice_state_pcm*)vstate;
    pcm_state->sample = -1;
    pcm_state->cents = 0;
    pcm_state->freq = 0;
    pcm_state->volume = 0;
    pcm_state->source = 0;
    pcm_state->expr = 0;
    pcm_state->middle_tone = 0;

    return;
}


uint32_t Proc_pcm_process_vstate(
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
    //assert(string_eq(proc->type, "pcm"));
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(audio_rate > 0);
    assert(tempo > 0);
    (void)proc_state;

//    Proc_pcm* pcm = (Proc_pcm*)proc->parent.dimpl;
    Voice_state_pcm* pcm_state = (Voice_state_pcm*)vstate;

    if (buf_start >= buf_stop)
        return buf_start;

    if (pcm_state->sample < 0)
    {
        // Select our sample

        int expression = 0;
        int source = 0;

        const int64_t* expression_arg = Channel_proc_state_get_int(
                vstate->cpstate, "e");
        if (expression_arg != NULL)
        {
            if (*expression_arg < 0 || *expression_arg >= PCM_EXPRESSIONS_MAX)
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
            if (*source_arg < 0 || *source_arg >= PCM_SOURCES_MAX)
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

            vstate->pitch = 440;
            entry = Hit_map_get_entry(
                    map,
                    vstate->hit_index,
                    vstate->force,
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
                    log2(vstate->pitch / 440) * 1200,
                    vstate->force,
                    vstate->rand_p);
            pcm_state->middle_tone = entry->ref_freq;
        }

        if (entry == NULL || entry->sample >= PCM_SAMPLES_MAX)
        {
            vstate->active = false;
            return buf_start;
        }

        pcm_state->sample = entry->sample;
        pcm_state->volume = entry->vol_scale;
        pcm_state->cents = entry->cents;
    }

    assert(pcm_state->sample < PCM_SAMPLES_MAX);

    // Find sample params
    char header_key[] = "smp_XXX/p_sh_sample.json";
    snprintf(
            header_key,
            strlen(header_key) + 1,
            "smp_%03x/p_sh_sample.json",
            pcm_state->sample);
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
             "smp_%03x/p_sample.%s", pcm_state->sample,
             extensions[header->format]);

    const Sample* sample = Device_params_get_sample(
            proc->parent.dparams, sample_key);
    if (sample == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    if (vstate->hit_index >= 0)
        pcm_state->middle_tone = 440;

    pcm_state->freq = header->mid_freq * exp2(pcm_state->cents / 1200);

    /*
    Sample_set_loop_start(sample, pcm_state->params.loop_start);
    Sample_set_loop_end(sample, pcm_state->params.loop_end);
    Sample_set_loop(sample, pcm_state->params.loop);
    // */

    return Sample_process_vstate(
            sample, header, vstate, wbs,
            buf_start, buf_stop, audio_rate, tempo,
            pcm_state->middle_tone, pcm_state->freq,
            pcm_state->volume);
}


void del_Proc_pcm(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_pcm* pcm = (Proc_pcm*)dimpl;
    memory_free(pcm);

    return;
}


