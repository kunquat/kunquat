

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
#include <stdbool.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_ins.h"
#include "Listener_ins_pcm.h"

#include <Song.h>
#include <Ins_table.h>
#include <Generator_pcm.h>
#include <Instrument.h>
#include <Song_limits.h>


int Listener_ins_pcm_load_sample(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    int32_t ins_num = argv[1]->i;
    Instrument* ins = NULL;
    if (!ins_get(lr, song_id, ins_num, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL,
            "The Instrument #%ld", (long)ins_num);
    Generator* gen = Instrument_get_gen(ins, 0);
    check_cond(lr, Generator_get_type(gen) == GEN_TYPE_PCM,
            "The Instrument type (%d)", (int)Generator_get_type(gen));
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    int32_t sample_index = argv[2]->i;
    check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
            "The Sample index (%ld)", (long)sample_index);
    char* spath = &argv[3]->s;
    if (!Generator_pcm_set_sample(gen_pcm, sample_index, spath))
    {
        lo_message m = new_msg();
        lo_message_add_string(m, "Couldn't load sample");
        lo_message_add_string(m, spath);
        int ret = 0;
        send_msg(lr, "error", m, ret);
        lo_message_free(m);
    }
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


int Listener_ins_pcm_sample_set_mid_freq(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    int32_t ins_num = argv[1]->i;
    Instrument* ins = NULL;
    if (!ins_get(lr, song_id, ins_num, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL,
            "The Instrument #%ld", (long)ins_num);
    Generator* gen = Instrument_get_gen(ins, 0);
    check_cond(lr, Generator_get_type(gen) == GEN_TYPE_PCM,
            "The Instrument type (%d)", (int)Generator_get_type(gen));
    int32_t sample_index = argv[2]->i;
    check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
            "The Sample index (%ld)", (long)sample_index);
    double freq = argv[3]->d;
    check_cond(lr, freq > 0, "The frequency (%f)", freq);
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    Generator_pcm_set_sample_freq(gen_pcm, sample_index, freq);
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


int Listener_ins_pcm_remove_sample(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    int32_t ins_num = argv[1]->i;
    Instrument* ins = NULL;
    if (!ins_get(lr, song_id, ins_num, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL,
            "The Instrument #%ld", (long)ins_num);
    Generator* gen = Instrument_get_gen(ins, 0);
    check_cond(lr, Generator_get_type(gen) == GEN_TYPE_PCM,
            "The Instrument type (%d)", (int)Generator_get_type(gen));
    int32_t sample_index = argv[2]->i;
    check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
            "The Sample index (%ld)", (long)sample_index);
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    if (!Generator_pcm_set_sample(gen_pcm, sample_index, NULL))
    {
        lo_message m = new_msg();
        lo_message_add_string(m, "Couldn't remove sample");
        lo_message_add_int32(m, sample_index);
        int ret = 0;
        send_msg(lr, "error", m, ret);
        lo_message_free(m);
    }
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


int Listener_ins_pcm_set_mapping(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    int32_t ins_num = argv[1]->i;
    Instrument* ins = NULL;
    if (!ins_get(lr, song_id, ins_num, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL,
            "The Instrument #%ld", (long)ins_num);
    Generator* gen = Instrument_get_gen(ins, 0);
    check_cond(lr, Generator_get_type(gen) == GEN_TYPE_PCM,
            "The Instrument type (%d)", (int)Generator_get_type(gen));
    int32_t source = argv[2]->i;
    check_cond(lr, source >= 0 && source < PCM_SOURCES_MAX,
            "The sound source (%ld)", (long)source);
    int32_t style = argv[3]->i;
    check_cond(lr, style >= 0 && style < PCM_STYLES_MAX,
            "The style index (%ld)", (long)style);
    int32_t strength = argv[4]->i;
    check_cond(lr, strength >= 0 && strength < PCM_FORCES_MAX,
            "The strength index (%ld)", (long)strength);
    double freq = argv[5]->d;
    check_cond(lr, freq > 0,
            "The lower-bound frequency (%f)", freq);
    int32_t index = argv[6]->i;
    check_cond(lr, index >= 0 && PCM_RANDOMS_MAX,
            "The random choice index (%ld)", (long)index);
    int32_t sample = argv[7]->i;
    check_cond(lr, sample >= 0 && sample < PCM_SAMPLES_MAX,
            "The Sample table index (%ld)", (long)sample);
    double freq_scale = argv[8]->d;
    check_cond(lr, freq_scale > 0,
            "The frequency scale factor (%f)", freq_scale);
    double vol_scale = argv[9]->d;
    check_cond(lr, vol_scale > 0,
            "The volume scale factor (%f)", vol_scale);
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    if (Generator_pcm_set_sample_mapping(gen_pcm,
            source, style, strength, freq, index,
            sample, freq_scale, vol_scale) < 0)
    {
        send_memory_fail(lr, "the Sample mapping");
    }
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


int Listener_ins_pcm_del_mapping(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data)
{
    (void)path;
    (void)types;
    (void)argc;
    (void)msg;
    assert(user_data != NULL);
    Listener* lr = user_data;
    if (lr->host == NULL)
    {
        return 0;
    }
    assert(lr->method_path != NULL);
    int32_t song_id = argv[0]->i;
    int32_t ins_num = argv[1]->i;
    Instrument* ins = NULL;
    if (!ins_get(lr, song_id, ins_num, &ins))
    {
        return 0;
    }
    check_cond(lr, ins != NULL,
            "The Instrument #%ld", (long)ins_num);
    Generator* gen = Instrument_get_gen(ins, 0);
    check_cond(lr, Generator_get_type(gen) == GEN_TYPE_PCM,
            "The Instrument type (%d)", (int)Generator_get_type(gen));
    int32_t source = argv[2]->i;
    check_cond(lr, source >= 0 && source < PCM_SOURCES_MAX,
            "The sound source (%ld)", (long)source);
    int32_t style = argv[3]->i;
    check_cond(lr, style >= 0 && style < PCM_STYLES_MAX,
            "The style index (%ld)", (long)style);
    int32_t strength = argv[4]->i;
    check_cond(lr, strength >= 0 && strength < PCM_FORCES_MAX,
            "The strength index (%ld)", (long)strength);
    double freq = argv[5]->d;
    check_cond(lr, freq > 0,
            "The lower-bound frequency (%f)", freq);
    int32_t index = argv[6]->i;
    check_cond(lr, index >= 0 && PCM_RANDOMS_MAX,
            "The random choice index (%ld)", (long)index);
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    Generator_pcm_del_sample_mapping(gen_pcm,
            source, style, strength, freq, index);
    ins_info(lr, song_id, ins_num, ins);
    return 0;
}


bool ins_info_pcm(Listener* lr, lo_message m, Instrument* ins)
{
    assert(lr != NULL);
    (void)lr;
    assert(m != NULL);
    assert(ins != NULL);
    Generator* gen = Instrument_get_gen(ins, 0);
    assert(Generator_get_type(gen) == GEN_TYPE_PCM);
    Generator_pcm* gen_pcm = (Generator_pcm*)gen;
    AAiter* iter = new_AAiter(gen_pcm->freq_maps[0].tree);
    if (iter == NULL)
    {
        send_memory_fail(lr, "the mapping iterator");
    }
    for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
    {
        if (Generator_pcm_get_sample(gen_pcm, i) == NULL)
        {
            continue;
        }
        lo_message_add_int32(m, i);
        lo_message_add_string(m, Generator_pcm_get_path(gen_pcm, i));
        lo_message_add_double(m,
                Generator_pcm_get_sample_freq(gen_pcm, i));
    }
    lo_message_add_string(m, "__styles");
    // TODO: send styles
    lo_message_add_string(m, "__maps");
    // TODO: send all maps
    lo_message_add_int32(m, 0); // source
    lo_message_add_int32(m, 0); // style
    lo_message_add_int32(m, 1); // # of strength levels

    lo_message_add_double(m, 0); // strength threshold
    lo_message_add_int32(m, gen_pcm->freq_maps[0].entry_count); // # of frequency levels

    freq_entry* key = &(freq_entry){ .freq = 0 };
    freq_entry* entry = AAiter_get(iter, key);
    while (entry != NULL)
    {
        lo_message_add_double(m, entry->freq);
        lo_message_add_int32(m, entry->choices);
        for (int i = 0; i < entry->choices; ++i)
        {
            lo_message_add_int32(m, entry->sample[i]);
            lo_message_add_double(m, entry->freq_scale[i]);
            lo_message_add_double(m, entry->vol_scale[i]);
        }
        entry = AAiter_get_next(iter);
    }
    del_AAiter(iter);
    
    return true;
}


