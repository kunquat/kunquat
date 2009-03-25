

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
#include <Instrument.h>
#include "Instrument_pcm.h"
#include <Sample.h>
#include <pitch_t.h>

#include <xmemory.h>


static freq_entry* new_freq_entry(pitch_t freq);

static int freq_entry_cmp(freq_entry* f1, freq_entry* f2);

static void del_freq_entry(freq_entry* entry);


int Instrument_pcm_init(Instrument* ins)
{
    assert(ins != NULL);
    pcm_type_data* type_data = xalloc(pcm_type_data);
    if (type_data == NULL)
    {
        return 1;
    }
    type_data->freq_maps[0].strength = 0;
    type_data->freq_maps[0].entry_count = 0;
    type_data->freq_maps[0].tree = new_AAtree(
            (int (*)(void*, void*))freq_entry_cmp, free);
    if (type_data->freq_maps[0].tree == NULL)
    {
        xfree(type_data);
        return 1;
    }
    type_data->freq_maps[0].strength = 0;
    for (uint8_t i = 1; i < PCM_STRENGTHS_MAX; ++i)
    {
        type_data->freq_maps[i].strength = 0;
        type_data->freq_maps[i].tree = NULL;
        type_data->freq_maps[i].entry_count = 0;
    }
    type_data->freq_map_count = 1;
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        type_data->samples[i] = NULL;
    }
    ins->type_data = type_data;
    if (Instrument_pcm_set_sample_mapping(ins,
            0, // source
            0, // style
            0, // strength id
            1, // ins freq
            0, // random choice index
            0, // sample table index
            1, // freq scale
            1) // vol scale
            < 0)
    {
        Instrument_pcm_uninit(ins);
        return 1;
    }
    return 0;
}


void Instrument_pcm_uninit(Instrument* ins)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    pcm_type_data* type_data = ins->type_data;
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        if (type_data->samples[i] != NULL)
        {
            del_Sample(type_data->samples[i]);
        }
    }
    del_AAtree(type_data->freq_maps[0].tree);
    xfree(ins->type_data);
    return;
}


bool Instrument_pcm_set_sample(Instrument* ins,
        uint16_t index,
        char* path)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(index < PCM_SAMPLES_MAX);
    pcm_type_data* type_data = ins->type_data;
    if (path == NULL)
    {
        if (type_data->samples[index] != NULL)
        {
            del_Sample(type_data->samples[index]);
            type_data->samples[index] = NULL;
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
    if (type_data->samples[index] != NULL)
    {
        del_Sample(type_data->samples[index]);
        type_data->samples[index] = NULL;
    }
    type_data->samples[index] = sample;
    return true;
}


Sample* Instrument_pcm_get_sample(Instrument* ins, uint16_t index)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(index < PCM_SAMPLES_MAX);
    pcm_type_data* type_data = ins->type_data;
    return type_data->samples[index];
}


char* Instrument_pcm_get_path(Instrument* ins, uint16_t index)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(index < PCM_SAMPLES_MAX);
    pcm_type_data* type_data = ins->type_data;
    if (type_data->samples[index] == NULL)
    {
        return NULL;
    }
    return Sample_get_path(type_data->samples[index]);
}


void Instrument_pcm_set_sample_freq(Instrument* ins,
        uint16_t index,
        double freq)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(index < PCM_SAMPLES_MAX);
    assert(freq > 0);
    pcm_type_data* type_data = ins->type_data;
    if (type_data->samples[index] == NULL)
    {
        return;
    }
    Sample_set_freq(type_data->samples[index], freq);
    return;
}


double Instrument_pcm_get_sample_freq(Instrument* ins, uint16_t index)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(index < PCM_SAMPLES_MAX);
    pcm_type_data* type_data = ins->type_data;
    if (type_data->samples[index] == NULL)
    {
        return 0;
    }
    return Sample_get_freq(type_data->samples[index]);
}


static Sample* state_to_sample(Instrument* ins, Voice_state* state);


void Instrument_pcm_mix(Instrument* ins,
        Voice_state* state,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(ins->bufs[0] != NULL);
    assert(ins->bufs[1] != NULL);
    if (!state->active)
    {
        return;
    }
    Sample* sample = state_to_sample(ins, state);
    if (sample == NULL)
    {
        return;
    }
    Sample_mix(sample, ins, state, nframes, offset, freq);
    return;
}


int8_t Instrument_pcm_set_sample_mapping(Instrument* ins,
        uint8_t source,
        uint8_t style,
        uint8_t strength_id,
        double ins_freq,
        uint8_t index,
        uint16_t sample, double freq_scale, double vol_scale)
{
    (void)source;
    (void)style;
    (void)strength_id;
    (void)index;
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(strength_id < PCM_STRENGTHS_MAX);
    assert(ins_freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    assert(sample < PCM_SAMPLES_MAX);
    assert(freq_scale > 0);
    assert(vol_scale > 0);
    pcm_type_data* type_data = ins->type_data;
    assert(type_data->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = ins_freq };
    freq_entry* entry = AAtree_get(type_data->freq_maps[0].tree, key, 1);
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
    if (!AAtree_ins(type_data->freq_maps[0].tree, entry))
    {
        xfree(entry);
        return -1;
    }
    ++type_data->freq_maps[0].entry_count;
    return 0;
}


bool Instrument_pcm_del_sample_mapping(Instrument* ins,
        uint8_t source,
        uint8_t style,
        uint8_t strength_id,
        double ins_freq,
        uint8_t index)
{
    (void)source;
    (void)style;
    (void)strength_id;
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(strength_id < PCM_STRENGTHS_MAX);
    assert(ins_freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    pcm_type_data* type_data = ins->type_data;
    assert(type_data->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = ins_freq };
    freq_entry* entry = AAtree_get(type_data->freq_maps[0].tree, key, 1);
    if (entry == NULL || index >= entry->choices)
    {
        return false;
    }
    freq_entry* ret = AAtree_remove(type_data->freq_maps[0].tree, entry);
    assert(ret == entry);
    del_freq_entry(ret);
    assert(type_data->freq_maps[0].entry_count > 0);
    --type_data->freq_maps[0].entry_count;
    return true;
}


static Sample* state_to_sample(Instrument* ins, Voice_state* state)
{
    assert(ins != NULL);
    assert(ins->type == INS_TYPE_PCM);
    assert(ins->type_data != NULL);
    assert(state != NULL);
    pcm_type_data* type_data = ins->type_data;
    assert(type_data->freq_maps[0].tree != NULL);
    freq_entry* key = &(freq_entry){ .freq = state->freq };
    freq_entry* entry = AAtree_get_at_most(type_data->freq_maps[0].tree, key, 0);
    if (entry == NULL)
    {
        return NULL;
    }
    assert(entry->choices > 0);
    int choice = rand() % entry->choices;
    assert(entry->sample[choice] < PCM_SAMPLES_MAX);
    return type_data->samples[entry->sample[choice]];
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


