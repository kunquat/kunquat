

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <AAtree.h>
#include <Generator.h>
#include <Generator_common.h>
#include <Generator_pcm.h>
#include <Voice_state_pcm.h>
#include <Sample.h>
#include <pitch_t.h>
#include <Parse_manager.h>
#include <File_wavpack.h>

#include <xmemory.h>


static int Random_list_cmp(const Random_list* list1, const Random_list* list2);

static void del_Random_list(Random_list* list);


static void Generator_pcm_init_state(Generator* gen, Voice_state* state);


static Sample_entry* state_to_sample(Generator_pcm* pcm, Voice_state_pcm* state);


Generator* new_Generator_pcm(Instrument_params* ins_params)
{
    assert(ins_params != NULL);
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
    pcm->parent.parse = Generator_pcm_parse;
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
    for (int i = 0; i < PCM_SOURCES_MAX * PCM_EXPRESSIONS_MAX; ++i)
    {
        pcm->maps[i] = NULL;
    }
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        Sample_params_init(&pcm->samples[i].params);
        for (Sample_format k = SAMPLE_FORMAT_NONE; k < SAMPLE_FORMAT_LAST; ++k)
        {
            pcm->samples[i].formats[k] = NULL;
        }
    }
    return &pcm->parent;
}


bool Generator_pcm_has_subkey(const char* subkey)
{
    assert(subkey != NULL);
    if (strncmp(subkey, "gen_pcm/", 8) != 0)
    {
        return false;
    }
    subkey = strchr(subkey, '/');
    assert(subkey != NULL);
    ++subkey;
    int sub_index = -1;
    if (parse_index_dir(subkey, "exp_", 1) >= 0)
    {
        const char* element = strchr(subkey, '/');
        assert(element != NULL);
        ++element;
        if (parse_index_dir(element, "src_", 1) >= 0)
        {
            element = strchr(element, '/');
            assert(element != NULL);
            ++element;
            if (strcmp(element, "p_sample_map.json") == 0)
            {
                return true;
            }
        }
    }
    else if ((sub_index = parse_index_dir(subkey, "smp_", 3)) >= 0 &&
            sub_index >= 0 && sub_index < PCM_SAMPLES_MAX)
    {
        const char* element = strchr(subkey, '/');
        assert(element != NULL);
        ++element;
        if (strcmp(element, "p_sample.json") == 0 ||
                strcmp(element, "p_sample.wv") == 0)
        {
            return true;
        }
    }
    return false;
}


static bool Generator_pcm_parse_sample_map(Generator* gen,
                                           char* str,
                                           Read_state* state,
                                           int expr,
                                           int source);


static bool Generator_pcm_parse_sample(Generator* gen,
                                       char* str,
                                       Read_state* state,
                                       int index);


static bool Generator_pcm_parse_wv(Generator* gen,
                                   void* data,
                                   long length,
                                   Read_state* state,
                                   int index);


bool Generator_pcm_parse(Generator* gen,
                         const char* subkey,
                         void* data,
                         long length,
                         Read_state* state)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_PCM);
    assert(subkey != NULL);
    assert(Generator_pcm_has_subkey(subkey));
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if (strncmp(subkey, "gen_pcm/", 8) != 0)
    {
        return false;
    }
    subkey = strchr(subkey, '/');
    assert(subkey != NULL);
    ++subkey;
    int sub_index = -1;
    if ((sub_index = parse_index_dir(subkey, "exp_", 1)) >= 0)
    {
        const char* element = strchr(subkey, '/');
        assert(element != NULL);
        ++element;
        int source_index = -1;
        if ((source_index = parse_index_dir(element, "src_", 1)) >= 0)
        {
            element = strchr(element, '/');
            assert(element != NULL);
            ++element;
            if (strcmp(element, "p_sample_map.json") == 0)
            {
                if (Generator_pcm_parse_sample_map(gen, data, state,
                                                   sub_index, source_index))
                {
                    return true;
                }
            }
        }
    }
    else if ((sub_index = parse_index_dir(subkey, "smp_", 3)) >= 0 &&
            sub_index >= 0 && sub_index < PCM_SAMPLES_MAX)
    {
        const char* element = strchr(subkey, '/');
        assert(element != NULL);
        ++element;
        if (strcmp(element, "p_sample.json") == 0)
        {
            if (Generator_pcm_parse_sample(gen, data, state, sub_index))
            {
                return true;
            }
        }
        else if (strcmp(element, "p_sample.wv") == 0)
        {
            if (Generator_pcm_parse_wv(gen, data, length, state, sub_index))
            {
                return true;
            }
        }
    }
    return false;
}


static AAtree* new_map_from_string(char* str, Read_state* state);


static bool Generator_pcm_parse_sample_map(Generator* gen,
                                           char* str,
                                           Read_state* state,
                                           int expr,
                                           int source)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_PCM);
    assert(state != NULL);
    assert(expr >= 0);
    assert(expr < PCM_EXPRESSIONS_MAX);
    assert(source >= 0);
    assert(source < PCM_SOURCES_MAX);
    if (state->error)
    {
        return false;
    }
    AAtree* map = new_map_from_string(str, state);
    if (map == NULL)
    {
        return false;
    }
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    int map_pos = (source * PCM_EXPRESSIONS_MAX) + expr;
    if (gen_pcm->maps[map_pos] != NULL)
    {
        del_AAtree(gen_pcm->maps[map_pos]);
    }
    gen_pcm->maps[map_pos] = map;
    return true;
}


static bool Generator_pcm_parse_sample(Generator* gen,
                                       char* str,
                                       Read_state* state,
                                       int index)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_PCM);
    assert(state != NULL);
    assert(index >= 0);
    assert(index < PCM_SAMPLES_MAX);
    if (state->error)
    {
        return false;
    }
    Sample_params* params = Sample_params_init(
            &(Sample_params){ .format = SAMPLE_FORMAT_NONE });
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            bool expect_key = true;
            while (expect_key)
            {
                char key[128] = { '\0' };
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (strcmp(key, "format") == 0)
                {
                    char format[32] = { '\0' };
                    str = read_string(str, format, 32, state);
                    if (!state->error)
                    {
                        if (strcmp(format, "WavPack") == 0)
                        {
                            params->format = SAMPLE_FORMAT_WAVPACK;
                        }
/*                        else if (strcmp(format, "Ogg Vorbis") == 0)
                        {
                            params->format = SAMPLE_FORMAT_VORBIS;
                        } */
                        else
                        {
                            Read_state_set_error(state,
                                    "Unrecognised Sample format: %s", format);
                        }
                    }
                }
                else if (strcmp(key, "mid_freq") == 0)
                {
                    str = read_double(str, &params->mid_freq, state);
                    if (!(params->mid_freq > 0))
                    {
                        Read_state_set_error(state,
                                "Sample frequency is not positive");
                    }
                }
                else if (strcmp(key, "loop_mode") == 0)
                {
                    char mode[] = "off!";
                    str = read_string(str, mode, 5, state);
                    if (strcmp(mode, "off") == 0)
                    {
                        params->loop = SAMPLE_LOOP_OFF;
                    }
                    else if (strcmp(mode, "uni") == 0)
                    {
                        params->loop = SAMPLE_LOOP_UNI;
                    }
                    else if (strcmp(mode, "bi") == 0)
                    {
                        params->loop = SAMPLE_LOOP_BI;
                    }
                    else
                    {
                        Read_state_set_error(state,
                                "Invalid Sample loop mode (must be"
                                " \"off\", \"uni\" or \"bi\")");
                    }
                }
                else if (strcmp(key, "loop_start") == 0)
                {
                    int64_t loop_start = 0;
                    str = read_int(str, &loop_start, state);
                    params->loop_start = loop_start;
                }
                else if (strcmp(key, "loop_end") == 0)
                {
                    int64_t loop_end = 0;
                    str = read_int(str, &loop_end, state);
                    params->loop_end = loop_end;
                }
                else
                {
                    Read_state_set_error(state,
                            "Unsupported key in Sample header: %s", key);
                }
                if (state->error)
                {
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    Sample_params_copy(&gen_pcm->samples[index].params, params);
    gen_pcm->samples[index].params.format = params->format;
    for (Sample_format i = SAMPLE_FORMAT_NONE + 1; i < SAMPLE_FORMAT_LAST; ++i)
    {
        if (gen_pcm->samples[index].formats[i] != NULL)
        {
            Sample_set_params(gen_pcm->samples[index].formats[i], params);
        }
    }
    return true;
}


static bool Generator_pcm_parse_wv(Generator* gen,
                                   void* data,
                                   long length,
                                   Read_state* state,
                                   int index)
{
    assert(gen != NULL);
    assert(Generator_get_type(gen) == GEN_TYPE_PCM);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    assert(index >= 0);
    assert(index < PCM_SAMPLES_MAX);
    if (state->error)
    {
        return false;
    }
    Sample* sample = new_Sample();
    if (sample == NULL)
    {
        return false;
    }
    if (!Sample_parse_wavpack(sample, data, length, state))
    {
        del_Sample(sample);
        return false;
    }
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    Sample_set_params(sample, &gen_pcm->samples[index].params);
    Generator_pcm_set_sample_of_type(gen_pcm, index, sample);
    return true;
}


static AAtree* new_map_from_string(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return NULL;
    }
    AAtree* map = new_AAtree((int (*)(const void*, const void*))Random_list_cmp,
                             (void (*)(void*))del_Random_list);
    if (map == NULL)
    {
        return NULL;
    }
    if (str == NULL)
    {
        return map;
    }
    str = read_const_char(str, '[', state);
    if (state->error)
    {
        del_AAtree(map);
        return NULL;
    }
    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        return map;
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
            check_next(str, state, expect_entry);
        }
        str = read_const_char(str, ']', state);

        str = read_const_char(str, ']', state);
        if (state->error)
        {
            del_AAtree(map);
            return NULL;
        }
        check_next(str, state, expect_list);
    }
    str = read_const_char(str, ']', state);
    if (state->error)
    {
        del_AAtree(map);
        return NULL;
    }
    return map;
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
    return;
}


void Generator_pcm_set_sample(Generator_pcm* gen_pcm,
                              uint16_t index,
                              Sample* sample)
{
    assert(gen_pcm != NULL);
    assert(Generator_get_type((Generator*)gen_pcm) == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(sample != NULL);
    Generator_pcm_set_sample_of_type(gen_pcm, index, sample);
    gen_pcm->samples[index].params.format = Sample_get_format(sample);
    return;
}


Sample* Generator_pcm_get_sample(Generator_pcm* gen_pcm, uint16_t index)
{
    assert(gen_pcm != NULL);
    assert(Generator_get_type((Generator*)gen_pcm) == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    Sample_format active_format = gen_pcm->samples[index].params.format;
    return gen_pcm->samples[index].formats[active_format];
}


void Generator_pcm_set_sample_of_type(Generator_pcm* gen_pcm,
                                      uint16_t index,
                                      Sample* sample)
{
    assert(gen_pcm != NULL);
    assert(Generator_get_type((Generator*)gen_pcm) == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(sample != NULL);
    Sample_format format = Sample_get_format(sample);
    if (gen_pcm->samples[index].formats[format] != NULL &&
            gen_pcm->samples[index].formats[format] != sample)
    {
        del_Sample(gen_pcm->samples[index].formats[format]);
    }
    gen_pcm->samples[index].formats[format] = sample;
    return;
}


Sample* Generator_pcm_get_sample_of_format(Generator_pcm* gen_pcm,
                                           uint16_t index,
                                           Sample_format format)
{
    assert(gen_pcm != NULL);
    assert(Generator_get_type((Generator*)gen_pcm) == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(format > SAMPLE_FORMAT_NONE);
    assert(format < SAMPLE_FORMAT_LAST);
    return gen_pcm->samples[index].formats[format];
}


char* Generator_pcm_get_path(Generator_pcm* pcm, uint16_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    Sample_format active_format = pcm->samples[index].params.format;
    if (pcm->samples[index].formats[active_format] == NULL)
    {
        return NULL;
    }
    return Sample_get_path(pcm->samples[index].formats[active_format]);
}


void Generator_pcm_set_sample_freq(Generator_pcm* pcm,
                                   uint16_t index,
                                   double freq)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    assert(freq > 0);
    Sample_format active_format = pcm->samples[index].params.format;
    if (pcm->samples[index].formats[active_format] == NULL)
    {
        return;
    }
    Sample_set_freq(pcm->samples[index].formats[active_format], freq);
    return;
}


double Generator_pcm_get_sample_freq(Generator_pcm* pcm, uint16_t index)
{
    assert(pcm != NULL);
    assert(pcm->parent.type == GEN_TYPE_PCM);
    assert(index < PCM_SAMPLES_MAX);
    Sample_format active_format = pcm->samples[index].params.format;
    if (pcm->samples[index].formats[active_format] == NULL)
    {
        return 0;
    }
    return Sample_get_freq(pcm->samples[index].formats[active_format]);
}


uint32_t Generator_pcm_mix(Generator* gen,
                           Voice_state* state,
                           uint32_t nframes,
                           uint32_t offset,
                           uint32_t freq,
                           double tempo,
                           int buf_count,
                           kqt_frame** bufs)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    assert(state != NULL);
//  assert(nframes <= ins->buf_len); XXX: Revisit after adding instrument buffers
    assert(freq > 0);
    assert(tempo > 0);
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
    Sample_format active_format = pcm->samples[pcm_state->sample].params.format;
    Sample* sample = pcm->samples[pcm_state->sample].formats[active_format];
    if (sample == NULL)
    {
        state->active = false;
        return offset;
    }
    return Sample_mix(sample, gen, state, nframes, offset, freq, tempo, buf_count, bufs,
                      pcm_state->middle_tone, pcm_state->freq);
}


void del_Generator_pcm(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->type == GEN_TYPE_PCM);
    Generator_pcm* pcm = (Generator_pcm*)gen;
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        for (Sample_format k = SAMPLE_FORMAT_NONE + 1; k < SAMPLE_FORMAT_LAST; ++k)
        {
            if (pcm->samples[i].formats[k] != NULL)
            {
                del_Sample(pcm->samples[i].formats[k]);
            }
        }
    }
    for (int i = 0; i < PCM_SOURCES_MAX * PCM_EXPRESSIONS_MAX; ++i)
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


static int Random_list_cmp(const Random_list* list1, const Random_list* list2)
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
    pitch_t pitch = state->parent.pitch;
    double force = state->parent.force;
//    fprintf(stderr, "searching list for %f Hz, %f dB... ", pitch, force);
    uint16_t source = state->source;
    uint16_t expr = state->expr;
    assert(pitch > 0);
    assert(isfinite(force));
    assert(source < PCM_SOURCES_MAX);
    assert(expr < PCM_EXPRESSIONS_MAX);
    int map_pos = (source * PCM_EXPRESSIONS_MAX) + expr;
    AAtree* map = pcm->maps[map_pos];
    if (map == NULL)
    {
//        fprintf(stderr, "no map\n");
        return NULL;
    }
    Random_list* key = &(Random_list){ .force = force,
                                       .freq = pitch,
                                       .freq_tone = log(pitch) };
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


