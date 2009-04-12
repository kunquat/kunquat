

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <AAtree.h>
#include <Generator.h>
#include <Generator_pcm.h>
#include <Sample.h>
#include <pitch_t.h>

#include <xmemory.h>


static freq_entry* new_freq_entry(pitch_t freq);

static int freq_entry_cmp(freq_entry* f1, freq_entry* f2);

static void del_freq_entry(freq_entry* entry);


Generator_pcm* new_Generator_pcm(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_pcm* pcm = xalloc(Generator_pcm);
    if (pcm == NULL)
    {
        return NULL;
    }
    pcm->parent.destroy = del_Generator_pcm;
    pcm->parent.type = GEN_TYPE_PCM;
    pcm->parent.init_state = NULL;
    pcm->parent.mix = Generator_pcm_mix;
    pcm->parent.ins_params = ins_params;
    pcm->freq_maps[0].force = 0;
    pcm->freq_maps[0].entry_count = 0;
    pcm->freq_maps[0].tree = new_AAtree(
            (int (*)(void*, void*))freq_entry_cmp, free);
    if (pcm->freq_maps[0].tree == NULL)
    {
        xfree(pcm);
        return NULL;
    }
    for (uint8_t i = 1; i < PCM_FORCES_MAX; ++i)
    {
        pcm->freq_maps[i].force = 0;
        pcm->freq_maps[i].tree = NULL;
        pcm->freq_maps[i].entry_count = 0;
    }
    pcm->freq_map_count = 1;
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        pcm->samples[i] = NULL;
    }
    if (Generator_pcm_set_sample_mapping(pcm,
            0, // source
            0, // style
            0, // force id
            1, // ins freq
            0, // random choice index
            0, // sample table index
            1, // freq scale
            1) // vol scale
            < 0)
    {
        del_Generator_pcm((Generator*)pcm);
        return NULL;
    }
    return pcm;
}


void del_Generator_pcm(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    Generator_pcm* pcm = (Generator_pcm*)gen;
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        if (pcm->samples[i] != NULL)
        {
            del_Sample(pcm->samples[i]);
        }
    }
    del_AAtree(pcm->freq_maps[0].tree);
    xfree(pcm);
    return;
}


bool Generator_pcm_set_sample(Generator_pcm* pcm,
        uint16_t index,
        char* path)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    if (path == NULL)
    {
        if (pcm->samples[index] != NULL)
        {
            del_Sample(pcm->samples[index]);
            pcm->samples[index] = NULL;
        }
        return true;
    }
    Sample* sample = new_Sample();
    if (sample == NULL)
    {
        return false;
    }
    if (!Sample_load_path(sample, path, SAMPLE_FORMAT_WAVPACK))
    {
        del_Sample(sample);
        return false;
    }
    if (pcm->samples[index] != NULL)
    {
        del_Sample(pcm->samples[index]);
        pcm->samples[index] = NULL;
    }
    pcm->samples[index] = sample;
    return true;
}


Sample* Generator_pcm_get_sample(Generator_pcm* pcm, uint16_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    return pcm->samples[index];
}


char* Generator_pcm_get_path(Generator_pcm* pcm, uint16_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    if (pcm->samples[index] == NULL)
    {
        return NULL;
    }
    return Sample_get_path(pcm->samples[index]);
}


void Generator_pcm_set_sample_freq(Generator_pcm* pcm,
        uint16_t index,
        double freq)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(freq > 0);
    if (pcm->samples[index] == NULL)
    {
        return;
    }
    Sample_set_freq(pcm->samples[index], freq);
    return;
}


double Generator_pcm_get_sample_freq(Generator_pcm* pcm, uint16_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    if (pcm->samples[index] == NULL)
    {
        return 0;
    }
    return Sample_get_freq(pcm->samples[index]);
}


static Sample* state_to_sample(Generator_pcm* pcm, Voice_state* state);


void Generator_pcm_mix(Generator* gen,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(gen->ins_params->bufs[0] != NULL);
    assert(gen->ins_params->bufs[1] != NULL);
    if (!state->active)
    {
        return;
    }
    Generator_pcm* pcm = (Generator_pcm*)gen;
    Sample* sample = state_to_sample(pcm, state);
    if (sample == NULL)
    {
        return;
    }
    Sample_mix(sample, gen, state, nframes, offset, freq);
    return;
}


int8_t Generator_pcm_set_sample_mapping(Generator_pcm* pcm,
        uint8_t source,
        uint8_t style,
        uint8_t force_id,
        double ins_freq,
        uint8_t index,
        uint16_t sample, double freq_scale, double vol_scale)
{
    (void)source;
    (void)style;
    (void)force_id;
    (void)index;
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(force_id < PCM_FORCES_MAX);
    assert(ins_freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    assert(sample < PCM_SAMPLES_MAX);
    assert(freq_scale > 0);
    assert(vol_scale > 0);
    assert(pcm->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = ins_freq };
    freq_entry* entry = AAtree_get(pcm->freq_maps[0].tree, key, 1);
    if (entry != NULL && entry->freq == ins_freq)
    {
        entry->sample[0] = sample;
        entry->freq_scale[0] = freq_scale;
        entry->vol_scale[0] = vol_scale;
        return 0;
    }
    entry = new_freq_entry(ins_freq);
    if (entry == NULL)
    {
        return -1;
    }
    entry->choices = 1;
    entry->sample[0] = sample;
    entry->freq_scale[0] = freq_scale;
    entry->vol_scale[0] = vol_scale;
    if (!AAtree_ins(pcm->freq_maps[0].tree, entry))
    {
        xfree(entry);
        return -1;
    }
    ++pcm->freq_maps[0].entry_count;
    return 0;
}


bool Generator_pcm_del_sample_mapping(Generator_pcm* pcm,
        uint8_t source,
        uint8_t style,
        uint8_t force_id,
        double ins_freq,
        uint8_t index)
{
    (void)source;
    (void)style;
    (void)force_id;
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(force_id < PCM_FORCES_MAX);
    assert(ins_freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    assert(pcm->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = ins_freq };
    freq_entry* entry = AAtree_get(pcm->freq_maps[0].tree, key, 1);
    if (entry == NULL || index >= entry->choices)
    {
        return false;
    }
    freq_entry* ret = AAtree_remove(pcm->freq_maps[0].tree, entry);
    assert(ret == entry);
    del_freq_entry(ret);
    assert(pcm->freq_maps[0].entry_count > 0);
    --pcm->freq_maps[0].entry_count;
    return true;
}


static Sample* state_to_sample(Generator_pcm* pcm, Voice_state* state)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(state != NULL);
    assert(pcm->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = state->freq };
    freq_entry* entry = AAtree_get_at_most(pcm->freq_maps[0].tree, key, 0);
    if (entry == NULL)
    {
        return NULL;
    }
    assert(entry->choices > 0);
    int choice = rand() % entry->choices;
    assert(entry->sample[choice] < PCM_SAMPLES_MAX);
    return pcm->samples[entry->sample[choice]];
}


static freq_entry* new_freq_entry(pitch_t freq)
{
    assert(freq > 0);
    freq_entry* entry = xalloc(freq_entry);
    if (entry == NULL)
    {
        return NULL;
    }
    entry->freq = freq;
    entry->choices = 0;
    for (uint8_t i = 0; i < PCM_RANDOMS_MAX; ++i)
    {
        entry->sample[i] = 0;
        entry->freq_scale[i] = 1;
        entry->vol_scale[i] = 1;
    }
    return entry;
}


static int freq_entry_cmp(freq_entry* f1, freq_entry* f2)
{
    assert(f1 != NULL);
    assert(f2 != NULL);
    if (f1->freq < f2->freq)
    {
        return -1;
    }
    else if (f1->freq > f2->freq)
    {
        return 1;
    }
    return 0;
}


static void del_freq_entry(freq_entry* entry)
{
    assert(entry != NULL);
    xfree(entry);
    return;
}


