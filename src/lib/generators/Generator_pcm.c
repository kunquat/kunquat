

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <AAtree.h>
#include <Generator.h>
#include <Generator_common.h>
#include <Generator_params.h>
#include <Generator_pcm.h>
#include <Voice_state_pcm.h>
#include <Sample.h>
#include <Sample_mix.h>
#include <pitch_t.h>
#include <Parse_manager.h>
#include <File_wavpack.h>
#include <xassert.h>
#include <xmemory.h>


static void Generator_pcm_init_state(Generator* gen, Voice_state* state);


Generator* new_Generator_pcm(Instrument_params* ins_params,
                             Generator_params* gen_params)
{
    assert(ins_params != NULL);
    assert(gen_params != NULL);
    Generator_pcm* pcm = xalloc(Generator_pcm);
    if (pcm == NULL)
    {
        return NULL;
    }
    if (!Generator_init(&pcm->parent))
    {
        xfree(pcm);
        return NULL;
    }
    pcm->parent.destroy = del_Generator_pcm;
    pcm->parent.type = GEN_TYPE_PCM;
    pcm->parent.init_state = Generator_pcm_init_state;
    pcm->parent.mix = Generator_pcm_mix;
    pcm->parent.ins_params = ins_params;
    pcm->parent.type_params = gen_params;
    return &pcm->parent;
}


static void Generator_pcm_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    (void)gen;
    assert(state != NULL);
    Voice_state_pcm* pcm_state = (Voice_state_pcm*)state;
    pcm_state->sample = -1;
    pcm_state->freq = 0;
    pcm_state->volume = 0;
    pcm_state->source = 0;
    pcm_state->expr = 0;
    pcm_state->middle_tone = 0;
    Sample_params_init(&pcm_state->params);
    return;
}


uint32_t Generator_pcm_mix(Generator* gen,
                           Voice_state* state,
                           uint32_t nframes,
                           uint32_t offset,
                           uint32_t freq,
                           double tempo)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(state != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    kqt_frame* bufs[] = { NULL, NULL };
    Generator_common_get_buffers(gen, state, offset, bufs);
    Generator_common_check_active(gen, state, offset);
//    Generator_pcm* pcm = (Generator_pcm*)gen;
    Voice_state_pcm* pcm_state = (Voice_state_pcm*)state;
    if (nframes <= offset)
    {
        return offset;
    }
    if (pcm_state->sample < 0)
    {
        int expression = 0;
        int source = 0;
        int64_t* expression_arg = Channel_gen_state_get_int(state->cgstate,
                                                            "expression.jsoni");
        if (expression_arg != NULL)
        {
            if (*expression_arg < 0 || *expression_arg >= PCM_EXPRESSIONS_MAX)
            {
                state->active = false;
                return offset;
            }
            expression = *expression_arg;
        }
        int64_t* source_arg = Channel_gen_state_get_int(state->cgstate,
                                                        "source.jsoni");
        if (source_arg != NULL)
        {
            if (*source_arg < 0 || *source_arg >= PCM_SOURCES_MAX)
            {
                state->active = false;
                return offset;
            }
            source = *source_arg;
        }
        char map_key[] = "exp_X/src_X/p_sample_map.jsonsm";
        snprintf(map_key, strlen(map_key) + 1,
                 "exp_%01x/src_%01x/p_sample_map.jsonsm", expression, source);
        Sample_map* map = Generator_params_get_sample_map(gen->type_params,
                                                          map_key);
        if (map == NULL)
        {
            state->active = false;
            return offset;
        }
        const Sample_entry* entry = Sample_map_get_entry(map,
                                            log2(state->pitch / 440) * 1200,
                                            state->force,
                                            gen->random);
        if (entry == NULL || entry->sample >= PCM_SAMPLES_MAX)
        {
            state->active = false;
            return offset;
        }
        pcm_state->sample = entry->sample;
        pcm_state->freq = entry->freq;
        pcm_state->volume = entry->vol_scale;
        pcm_state->middle_tone = entry->ref_freq;
        char header_key[] = "smp_XXX/p_sample.jsonsh";
        snprintf(header_key, strlen(header_key) + 1,
                 "smp_%03x/p_sample.jsonsh", pcm_state->sample);
        Sample_params* header = Generator_params_get_sample_params(gen->type_params,
                                        header_key);
        if (header == NULL)
        {
            state->active = false;
            return offset;
        }
        Sample_params_copy(&pcm_state->params, header);
    }
    assert(pcm_state->sample < PCM_SAMPLES_MAX);
    assert(pcm_state->params.format > SAMPLE_FORMAT_NONE);
    static const char* extensions[] =
    {
        [SAMPLE_FORMAT_WAVPACK] = "wv",
    };
    char sample_key[] = "smp_XXX/p_sample.NONE";
    snprintf(sample_key, strlen(sample_key) + 1,
             "smp_%03x/p_sample.%s", pcm_state->sample,
             extensions[pcm_state->params.format]);
    Sample* sample = Generator_params_get_sample(gen->type_params, sample_key);
    if (sample == NULL)
    {
        state->active = false;
        return offset;
    }
    Sample_set_loop_start(sample, pcm_state->params.loop_start);
    Sample_set_loop_end(sample, pcm_state->params.loop_end);
    Sample_set_loop(sample, pcm_state->params.loop);
    return Sample_mix(sample, gen, state, nframes, offset, freq, tempo, 2, bufs,
                      pcm_state->middle_tone, pcm_state->freq);
}


void del_Generator_pcm(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    Generator_pcm* pcm = (Generator_pcm*)gen;
    Generator_uninit(gen);
    xfree(pcm);
    return;
}


#if 0
int8_t Generator_pcm_set_sample_mapping(Generator_pcm* pcm,
                                        uint8_t source,
                                        uint8_t expr,
                                        double force,
                                        double freq,
                                        uint8_t index,
                                        uint16_t sample,
                                        double sample_freq,
                                        double vol_scale)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(expr < PCM_EXPRESSIONS_MAX);
    assert(isfinite(force));
    assert(freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    assert(sample < PCM_SAMPLES_MAX);
    assert(sample_freq > 0);
    assert(vol_scale >= 0);
    int map_pos = (source * PCM_EXPRESSIONS_MAX) + expr;
    AAtree* map = pcm->maps[map_pos];
    AAtree* new_map = NULL;
    if (map == NULL)
    {
        new_map = new_AAtree((int (*)(const void*, const void*))Random_list_cmp,
                         (void (*)(void*))del_Random_list);
        if (new_map == NULL)
        {
            return -1;
        }
        pcm->maps[map_pos] = map = new_map;
    }
    Random_list* key = &(Random_list){ .force = force, .freq = freq };
    Random_list* list = AAtree_get_exact(map, key);
    Random_list* new_list = NULL;
    if (list == NULL)
    {
        new_list = xalloc(Random_list);
        if (new_list == NULL)
        {
            if (new_map != NULL)
            {
                pcm->maps[map_pos] = NULL;
                del_AAtree(new_map);
            }
            return -1;
        }
        new_list->force = force;
        new_list->freq = freq;
        new_list->freq_tone = log(freq);
        new_list->entry_count = 0;
        for (int i = 0; i < PCM_RANDOMS_MAX; ++i)
        {
            new_list->entries[i].freq = -1;
        }
        if (!AAtree_ins(map, new_list))
        {
            if (new_map != NULL)
            {
                pcm->maps[map_pos] = NULL;
                del_AAtree(new_map);
                return -1;
            }
        }
        list = new_list;
    }
    if (index >= list->entry_count)
    {
        index = list->entry_count;
        ++list->entry_count;
    }
    list->entries[index].freq = sample_freq;
    list->entries[index].vol_scale = vol_scale;
    list->entries[index].sample = sample;
    return index;
}
#endif


#if 0
bool Generator_pcm_del_sample_mapping(Generator_pcm* pcm,
                                      uint8_t source,
                                      uint8_t expr,
                                      double force,
                                      double freq,
                                      uint8_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(expr < PCM_EXPRESSIONS_MAX);
    assert(isfinite(force));
    assert(isfinite(freq));
    assert(index < PCM_RANDOMS_MAX);
    int map_pos = (source * PCM_EXPRESSIONS_MAX) + expr;
    AAtree* map = pcm->maps[map_pos];
    if (map == NULL)
    {
        return false;
    }
    Random_list* key = &(Random_list){ .force = force, .freq = freq };
    Random_list* list = AAtree_get_exact(map, key);
    if (list == NULL)
    {
        return false;
    }
    if (index >= list->entry_count)
    {
        return false;
    }
    while (index < PCM_RANDOMS_MAX - 1 && list->entries[index].freq > 0)
    {
        list->entries[index].freq = list->entries[index + 1].freq;
        list->entries[index].vol_scale = list->entries[index + 1].vol_scale;
        list->entries[index].sample = list->entries[index + 1].sample;
        ++index;
    }
    list->entries[index].freq = -1;
    --list->entry_count;
    return true;
}
#endif


