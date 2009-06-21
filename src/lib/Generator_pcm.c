

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
#include <Generator_common.h>
#include <Generator_pcm.h>
#include <Voice_state_pcm.h>
#include <Sample.h>
#include <pitch_t.h>

#include <xmemory.h>


static int Random_list_cmp(Random_list* list1, Random_list* list2);

static void del_Random_list(Random_list* list);


static bool Generator_pcm_read(Generator* gen, File_tree* tree, Read_state* state);

static void Generator_pcm_init_state(Generator* gen, Voice_state* state);


static Sample_entry* state_to_sample(Generator_pcm* pcm, Voice_state_pcm* state);


Generator_pcm* new_Generator_pcm(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
    Generator_pcm* pcm = xalloc(Generator_pcm);
    if (pcm == NULL)
    {
        return NULL;
    }
    Generator_init(&pcm->parent);
    pcm->parent.read = Generator_pcm_read;
    pcm->parent.destroy = del_Generator_pcm;
    pcm->parent.type = GEN_TYPE_PCM;
    pcm->parent.init_state = Generator_pcm_init_state;
    pcm->parent.mix = Generator_pcm_mix;
    pcm->parent.ins_params = ins_params;
    pcm->iter = new_AAiter(NULL);
    if (pcm->iter == NULL)
    {
        xfree(pcm);
        return NULL;
    }
    for (int i = 0; i < PCM_SOURCES_MAX * PCM_STYLES_MAX; ++i)
    {
        pcm->maps[i] = NULL;
    }
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        pcm->samples[i] = NULL;
    }
    return pcm;
}


static AAtree* new_map_from_file_tree(File_tree* tree, Read_state* state);


static AAtree* new_map_from_file_tree(File_tree* tree, Read_state* state)
{
    assert(tree != NULL);
    assert(!File_tree_is_dir(tree));
    if (state->error)
    {
        return NULL;
    }
    AAtree* map = new_AAtree((int (*)(void*, void*))Random_list_cmp,
                             (void (*)(void*))del_Random_list);
    if (map == NULL)
    {
        return NULL;
    }
    char* str = File_tree_get_data(tree);
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_AAtree(map);
        Read_state_set_error(state, "Couldn't allocate memory for sample mapping");
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        del_AAtree(map);
        return NULL;
    }
    Read_state_clear_error(state);
    bool expect_list = true;
    while (expect_list)
    {
        Random_list* list = xalloc(Random_list);
        if (list == NULL)
        {
            Read_state_set_error(state, "Couldn't allocate memory for sample point");
            del_AAtree(map);
            return NULL;
        }
        str = read_const_char(str, '[', state);

        str = read_const_char(str, '[', state);
        double freq = NAN;
        str = read_double(str, &freq, state);
        str = read_const_char(str, ',', state);
        double force = NAN;
        str = read_double(str, &force, state);
        str = read_const_char(str, ']', state);
        str = read_const_char(str, ',', state);
        str = read_const_char(str, '[', state);
        if (state->error)
        {
            xfree(list);
            del_AAtree(map);
            return NULL;
        }
        if (!(freq > 0))
        {
            Read_state_set_error(state, "Mapping frequency is not positive");
            xfree(list);
            del_AAtree(map);
            return NULL;
        }
        if (!isfinite(force))
        {
            Read_state_set_error(state, "Mapping force is not finite");
            xfree(list);
            del_AAtree(map);
            return NULL;
        }
        if (!AAtree_ins(map, list))
        {
            Read_state_set_error(state, "Couldn't allocate memory for sample mapping");
            xfree(list);
            del_AAtree(map);
            return NULL;
        }
        list->freq = freq;
        list->freq_tone = log(freq);
        list->force = force;
        list->entry_count = 0;
        bool expect_entry = true;
        while (expect_entry && list->entry_count < PCM_RANDOMS_MAX)
        {
            str = read_const_char(str, '[', state);
            double sample_freq = NAN;
            str = read_double(str, &sample_freq, state);
            str = read_const_char(str, ',', state);
            double vol_scale = NAN;
            str = read_double(str, &vol_scale, state);
            str = read_const_char(str, ',', state);
            int64_t sample = -1;
            str = read_int(str, &sample, state);
            str = read_const_char(str, ']', state);
            if (state->error)
            {
                del_AAtree(map);
                return NULL;
            }
            if (!(sample_freq > 0))
            {
                Read_state_set_error(state, "Sample frequency is not positive");
                del_AAtree(map);
                return NULL;
            }
            if (!(vol_scale >= 0))
            {
                Read_state_set_error(state, "Volume scale is not positive or zero");
                del_AAtree(map);
                return NULL;
            }
            if (sample < 0 || sample > PCM_SAMPLES_MAX)
            {
                Read_state_set_error(state, "Sample number is outside valid range [0..%d]",
                                     PCM_SAMPLES_MAX - 1);
                del_AAtree(map);
                return NULL;
            }
            list->entries[list->entry_count].freq = sample_freq;
            list->entries[list->entry_count].vol_scale = vol_scale;
            list->entries[list->entry_count].sample = sample;
            ++list->entry_count;
            str = read_const_char(str, ',', state);
            if (state->error)
            {
                Read_state_clear_error(state);
                expect_entry = false;
            }
        }
        str = read_const_char(str, ']', state);

        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_AAtree(map);
            return NULL;
        }
        str = read_const_char(str, ',', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            expect_list = false;
        }
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_AAtree(map);
        return NULL;
    }
    return map;
}


static bool Generator_pcm_read(Generator* gen, File_tree* tree, Read_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(tree != NULL);
    assert(File_tree_is_dir(tree));
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Read_state_init(state, File_tree_get_path(tree));
    Generator_pcm* pcm = (Generator_pcm*)gen;
    for (int i = 0; i < PCM_STYLES_MAX; ++i)
    {
        char st_name[] = "st_XX";
        snprintf(st_name, 5, "st_%01x", i);
        File_tree* style_tree = File_tree_get_child(tree, st_name);
        if (style_tree != NULL)
        {
            Read_state_init(state, File_tree_get_path(style_tree));
            if (!File_tree_is_dir(style_tree))
            {
                Read_state_set_error(state, "PCM Generator style description %01x"
                                            " is not a directory", i);
                return false;
            }
            for (int k = 0; k < PCM_SOURCES_MAX; ++k)
            {
                char src_name[] = "src_XX";
                snprintf(src_name, 6, "src_%01x", k);
                File_tree* source_tree = File_tree_get_child(style_tree, src_name);
                if (source_tree != NULL)
                {
                    Read_state_init(state, File_tree_get_path(source_tree));
                    if (!File_tree_is_dir(source_tree))
                    {
                        Read_state_set_error(state, "PCM Generator source description %01x:%01x"
                                                    " is not a directory", i, k);
                        return false;
                    }
                    File_tree* map_tree = File_tree_get_child(source_tree, "map.json");
                    if (map_tree != NULL)
                    {
                        if (File_tree_is_dir(map_tree))
                        {
                            Read_state_set_error(state, "PCM Generator sample mapping %01x:%01x"
                                                       " is a directory", i, k);
                            return false;
                        }
                        AAtree* map = new_map_from_file_tree(map_tree, state);
                        if (state->error)
                        {
                            assert(map == NULL);
                            return false;
                        }
                        pcm->maps[(k * PCM_STYLES_MAX) + i] = map;
                    }
                }
            }
        }
    }
    for (int i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        char dir_name[] = "sample_XXX";
        snprintf(dir_name, 11, "sample_%03x", i);
        File_tree* sample_tree = File_tree_get_child(tree, dir_name);
        if (sample_tree != NULL)
        {
            Read_state_init(state, File_tree_get_path(sample_tree));
            Sample* sample = new_Sample_from_file_tree(sample_tree, state);
            if (state->error)
            {
                assert(sample == NULL);
                return false;
            }
            pcm->samples[i] = sample;
        }
    }
    return true;
}


static void Generator_pcm_init_state(Generator* gen, Voice_state* state)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(state != NULL);
    Voice_state_init(state);
    Voice_state_pcm* pcm_state = (Voice_state_pcm*)state;
    pcm_state->sample = -1;
    pcm_state->freq = 0;
    pcm_state->volume = 0;
    pcm_state->source = 0;
    pcm_state->style = 0;
    pcm_state->middle_tone = 0;
    return;
}


void Generator_pcm_set_sample(Generator_pcm* pcm,
        uint16_t index,
        Sample* sample)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(sample != NULL);
    if (pcm->samples[index] != NULL)
    {
        del_Sample(pcm->samples[index]);
        pcm->samples[index] = NULL;
    }
    pcm->samples[index] = sample;
    return;
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


uint32_t Generator_pcm_mix(Generator* gen,
                           Voice_state* state,
                           uint32_t nframes,
                           uint32_t offset,
                           uint32_t freq,
                           int buf_count,
                           frame_t** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(buf_count > 0);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    Generator_common_check_active(gen, state, offset);
    Generator_pcm* pcm = (Generator_pcm*)gen;
    Voice_state_pcm* pcm_state = (Voice_state_pcm*)state;
    if (nframes <= offset)
    {
        return offset;
    }
    if (pcm_state->sample < 0)
    {
        Sample_entry* entry = state_to_sample(pcm, pcm_state);
        if (entry == NULL)
        {
            state->active = false;
            return offset;
        }
        pcm_state->sample = entry->sample;
        pcm_state->freq = entry->freq;
        pcm_state->volume = entry->vol_scale;
    }
    assert(pcm_state->sample < PCM_SAMPLES_MAX);
    Sample* sample = pcm->samples[pcm_state->sample];
    if (sample == NULL)
    {
        state->active = false;
        return offset;
    }
    return Sample_mix(sample, gen, state, nframes, offset, freq, buf_count, bufs,
                      pcm_state->middle_tone, pcm_state->freq);
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
    for (int i = 0; i < PCM_SOURCES_MAX * PCM_STYLES_MAX; ++i)
    {
        if (pcm->maps[i] != NULL)
        {
            del_AAtree(pcm->maps[i]);
            pcm->maps[i] = NULL;
        }
    }
    del_AAiter(pcm->iter);
    xfree(pcm);
    return;
}


static int Random_list_cmp(Random_list* list1, Random_list* list2)
{
    assert(list1 != NULL);
    assert(list2 != NULL);
    if (list1->freq < list2->freq)
    {
        return -1;
    }
    else if (list1->freq > list2->freq)
    {
        return 1;
    }
    if (list1->force < list2->force)
    {
        return -1;
    }
    if (list1->force < list2->force)
    {
        return 1;
    }
    return 0;
}


static void del_Random_list(Random_list* list)
{
    assert(list != NULL);
    xfree(list);
    return;
}


int8_t Generator_pcm_set_sample_mapping(Generator_pcm* pcm,
        uint8_t source, uint8_t style, double force, double freq, uint8_t index,
        uint16_t sample, double sample_freq, double vol_scale)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(isfinite(force));
    assert(freq > 0);
    assert(index < PCM_RANDOMS_MAX);
    assert(sample < PCM_SAMPLES_MAX);
    assert(sample_freq > 0);
    assert(vol_scale >= 0);
    int map_pos = (source * PCM_STYLES_MAX) + style;
    AAtree* map = pcm->maps[map_pos];
    AAtree* new_map = NULL;
    if (map == NULL)
    {
        new_map = new_AAtree((int (*)(void*, void*))Random_list_cmp,
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


bool Generator_pcm_del_sample_mapping(Generator_pcm* pcm,
                                      uint8_t source,
                                      uint8_t style,
                                      double force,
                                      double freq,
                                      uint8_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    assert(isfinite(force));
    assert(isfinite(freq));
    assert(index < PCM_RANDOMS_MAX);
    int map_pos = (source * PCM_STYLES_MAX) + style;
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


static double distance(Random_list* list, Random_list* key);


static double distance(Random_list* list, Random_list* key)
{
    assert(list != NULL);
    assert(key != NULL);
    double tone_d = (list->freq_tone - key->freq_tone) * 64;
    double force_d = list->force - key->force;
    return sqrt(tone_d * tone_d + force_d * force_d);
}


static Sample_entry* state_to_sample(Generator_pcm* pcm, Voice_state_pcm* state)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(state != NULL);
    pitch_t freq = state->parent.freq;
    double force = state->parent.force;
//    fprintf(stderr, "searching list for %f Hz, %f dB... ", freq, force);
    uint8_t source = state->source;
    uint8_t style = state->style;
    assert(freq > 0);
    assert(isfinite(force));
    assert(source < PCM_SOURCES_MAX);
    assert(style < PCM_STYLES_MAX);
    int map_pos = (source * PCM_STYLES_MAX) + style;
    AAtree* map = pcm->maps[map_pos];
    if (map == NULL)
    {
//        fprintf(stderr, "no map\n");
        return NULL;
    }
    Random_list* key = &(Random_list){ .force = force,
                                       .freq = freq,
                                       .freq_tone = log(freq) };
    AAiter_change_tree(pcm->iter, map);
    Random_list* estimate_low = AAiter_get_at_most(pcm->iter, key);
    Random_list* choice = NULL;
    double choice_d = INFINITY;
    if (estimate_low != NULL)
    {
        choice = estimate_low;
        choice_d = distance(choice, key);
        double min_tone = key->freq_tone - choice_d;
        Random_list* candidate = AAiter_get_prev(pcm->iter);
        while (candidate != NULL && candidate->freq_tone >= min_tone)
        {
            double d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                min_tone = key->freq_tone - choice_d;
            }
            candidate = AAiter_get_prev(pcm->iter);
        }
    }
    Random_list* estimate_high = AAiter_get(pcm->iter, key);
    if (estimate_high != NULL)
    {
        double d = distance(estimate_high, key);
        if (choice == NULL || choice_d > d)
        {
            choice = estimate_high;
            choice_d = d;
        }
        double max_tone = key->freq_tone + choice_d;
        Random_list* candidate = AAiter_get_next(pcm->iter);
        while (candidate != NULL && candidate->freq_tone <= max_tone)
        {
            d = distance(candidate, key);
            if (d < choice_d)
            {
                choice = candidate;
                choice_d = d;
                max_tone = key->freq_tone + choice_d;
            }
            candidate = AAiter_get_next(pcm->iter);
        }
    }
    if (choice == NULL)
    {
//        fprintf(stderr, "empty map\n");
        return NULL;
    }
    assert(choice->entry_count > 0);
    assert(choice->entry_count < PCM_RANDOMS_MAX);
    state->middle_tone = choice->freq;
    int index = (rand() >> 8) % choice->entry_count;
    assert(index >= 0);
//    fprintf(stderr, "%d\n", index);
    return &choice->entries[index];
}


