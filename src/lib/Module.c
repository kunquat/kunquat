

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <math.h>

#include <Connections_search.h>
#include <File_base.h>
#include <math_common.h>
#include <memory.h>
#include <Random.h>
#include <Real.h>
#include <Module.h>
#include <string_common.h>
#include <xassert.h>


/**
 * Resets the Module.
 *
 * \param device   The Module Device -- must not be \c NULL.
 */
static void Module_reset(Device* device, Device_states* dstates);


/**
 * Sets the master random seed of the Module.
 *
 * \param module   The Module -- must not be \c NULL.
 * \param seed   The random seed.
 */
static void Module_set_random_seed(Module* module, uint64_t seed);


static bool Module_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t audio_rate);


static bool Module_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size);


static void Module_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo);


Module* new_Module()
{
    Module* module = memory_alloc_item(Module);
    if (module == NULL)
        return NULL;

    if (!Device_init(&module->parent))
    {
        memory_free(module);
        return NULL;
    }

    Device_set_existent(&module->parent, true);
    Device_set_reset(&module->parent, Module_reset);
    Device_register_set_audio_rate(&module->parent, Module_set_audio_rate);
    Device_register_update_tempo(&module->parent, Module_update_tempo);
    Device_register_set_buffer_size(&module->parent, Module_set_buffer_size);
    Device_register_port(&module->parent, DEVICE_PORT_TYPE_RECEIVE, 0);

    // Clear fields
    module->songs = NULL;
    module->pats = NULL;
    module->ins_map = NULL;
    module->insts = NULL;
    module->effects = NULL;
    module->connections = NULL;
    module->random = NULL;
    module->env = NULL;
    module->bind = NULL;
    module->album_is_existent = false;
    module->track_list = NULL;
    for (int i = 0; i < KQT_SONGS_MAX; ++i)
        module->order_lists[i] = NULL;
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
        module->scales[i] = NULL;

    // Create fields
    module->random = new_Random();
    module->songs = new_Song_table();
    module->pats = new_Pat_table(KQT_PATTERNS_MAX);
    module->insts = new_Ins_table(KQT_INSTRUMENTS_MAX);
    module->effects = new_Effect_table(KQT_EFFECTS_MAX);
    if (module->random == NULL       ||
            module->songs == NULL    ||
            module->pats == NULL     ||
            module->insts == NULL    ||
            module->effects == NULL)
    {
        del_Module(module);
        return NULL;
    }

    module->scales[0] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (module->scales[0] == NULL)
    {
        del_Module(module);
        return NULL;
    }

    module->env = new_Environment();
    if (module->env == NULL)
    {
        del_Module(module);
        return NULL;
    }

    Streader* conn_sr = Streader_init(STREADER_AUTO, NULL, 0);
    module->connections = new_Connections_from_string(
            conn_sr,
            false,
            module->insts,
            module->effects,
            NULL,
            &module->parent);
    if (module->connections == NULL)
    {
        del_Module(module);
        return NULL;
    }

    if (Scale_ins_note(module->scales[0], 0,
                       Real_init_as_frac(REAL_AUTO, 1, 1)) < 0)
    {
        del_Module(module);
        return NULL;
    }

    for (int i = 1; i < 12; ++i)
    {
        if (Scale_ins_note_cents(module->scales[0], i, i * 100) < 0)
        {
            del_Module(module);
            return NULL;
        }
    }

    module->mix_vol_dB = MODULE_DEFAULT_MIX_VOL;
    module->mix_vol = exp2(module->mix_vol_dB / 6);
    //module->init_subsong = SONG_DEFAULT_INIT_SUBSONG;
    Module_set_random_seed(module, 0);

    return module;
}


typedef struct mod_params
{
    double mix_vol;
} mod_params;

static bool read_mod_param(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    mod_params* mp = userdata;

    if (string_eq(key, "mix_vol"))
    {
        if (!Streader_read_float(sr, &mp->mix_vol))
            return false;

        if (!isfinite(mp->mix_vol) && mp->mix_vol != -INFINITY)
        {
            Streader_set_error(sr,
                     "Invalid mixing volume: %f", mp->mix_vol);
            return false;
        }
    }
    else
    {
        Streader_set_error(sr,
                 "Unrecognised key in composition info: %s", key);
        return false;
    }

    return true;
}

bool Module_parse_composition(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    mod_params* mp = &(mod_params)
    {
        .mix_vol = MODULE_DEFAULT_MIX_VOL,
    };

    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_mod_param, mp))
            return false;
    }

    module->mix_vol_dB = mp->mix_vol;
    module->mix_vol = exp2(module->mix_vol_dB / 6);

    return true;
}


bool Module_parse_random_seed(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    if (!Streader_is_error_set(sr))
        return false;

    int64_t seed = 0;

    if (Streader_has_data(sr))
    {
        if (!Streader_read_int(sr, &seed))
            return false;

        if (seed < 0)
        {
            Streader_set_error(sr, "Random seed must be positive");
            return false;
        }
    }

    Module_set_random_seed(module, seed);

    return true;
}


const Track_list* Module_get_track_list(const Module* module)
{
    assert(module != NULL);

    if (!module->album_is_existent)
        return NULL;

    assert(module->track_list != NULL);
    return module->track_list;
}


const Order_list* Module_get_order_list(const Module* module, int16_t song)
{
    assert(module != NULL);
    assert(song >= 0);
    assert(song < KQT_SONGS_MAX);

    if (!Song_table_get_existent(module->songs, song))
        return NULL;

    assert(module->order_lists[song] != NULL);
    return module->order_lists[song];
}


const Pattern* Module_get_pattern(
        const Module* module,
        const Pat_inst_ref* piref)
{
    assert(module != NULL);
    assert(piref != NULL);
    assert(piref->pat >= 0);
    assert(piref->pat < KQT_PATTERNS_MAX);

    if (!Pat_table_get_existent(module->pats, piref->pat))
        return NULL;

    const Pattern* pat = Pat_table_get(module->pats, piref->pat);
    if (pat != NULL && !Pattern_get_inst_existent(pat, piref->inst))
        return NULL;

    return pat;
}


bool Module_find_pattern_location(
        const Module* module,
        const Pat_inst_ref* piref,
        int16_t* track,
        int16_t* system)
{
    assert(module != NULL);
    assert(piref != NULL);
    assert(piref->pat >= 0);
    assert(piref->pat < KQT_PATTERNS_MAX);
    assert(piref->inst >= 0);
    assert(piref->inst < KQT_PAT_INSTANCES_MAX);
    assert(track != NULL);
    assert(system != NULL);

    // Linear search all track lists
    for (int ti = 0; ti < KQT_SONGS_MAX; ++ti)
    {
        if (!Song_table_get_existent(module->songs, ti))
            continue;

        const Order_list* ol = module->order_lists[ti];
        assert(ol != NULL);

        for (size_t i = 0; i < Order_list_get_len(ol); ++i)
        {
            const Pat_inst_ref* cur_piref = Order_list_get_pat_inst_ref(ol, i);
            assert(cur_piref != NULL);

            if (cur_piref->pat == piref->pat && cur_piref->inst == piref->inst)
            {
                *track = ti;
                *system = i;
                return true;
            }
        }
    }

    return false;
}


void Module_set_mix_vol(Module* module, double mix_vol)
{
    assert(module != NULL);
    assert(isfinite(mix_vol) || mix_vol == -INFINITY);

    module->mix_vol_dB = mix_vol;
    module->mix_vol = exp2(mix_vol / 6);

    return;
}


double Module_get_mix_vol(Module* module)
{
    assert(module != NULL);
    return module->mix_vol_dB;
}


Song_table* Module_get_songs(const Module* module)
{
    assert(module != NULL);
    return module->songs;
}


Pat_table* Module_get_pats(Module* module)
{
    assert(module != NULL);
    return module->pats;
}


Instrument* Module_get_ins_from_input(const Module* module, int32_t input)
{
    assert(module != NULL);
    assert(input >= 0);

    if (module->ins_map == NULL)
        return NULL;

    int32_t ins_index = Input_map_get_device_index(module->ins_map, input);
    if (ins_index < 0)
        return NULL;

    assert(ins_index < KQT_INSTRUMENTS_MAX);
    return Ins_table_get(module->insts, ins_index);
}


bool Module_set_ins_map(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    Input_map* im = new_Input_map(sr, INT32_MAX, KQT_INSTRUMENTS_MAX);
    if (im == NULL)
        return false;

    del_Input_map(module->ins_map);
    module->ins_map = im;

    return true;
}


Input_map* Module_get_ins_map(const Module* module)
{
    assert(module != NULL);
    return module->ins_map;
}


Ins_table* Module_get_insts(const Module* module)
{
    assert(module != NULL);
    return module->insts;
}


Effect_table* Module_get_effects(const Module* module)
{
    assert(module != NULL);
    return module->effects;
}


void Module_set_bind(Module* module, Bind* bind)
{
    assert(module != NULL);
    assert(bind != NULL);

    del_Bind(module->bind);
    module->bind = bind;

    return;
}


Scale* Module_get_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    return module->scales[index];
}


void Module_set_scale(Module* module, int index, Scale* scale)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    assert(scale != NULL);

    if (module->scales[index] != NULL &&
            module->scales[index] != scale)
        del_Scale(module->scales[index]);

    module->scales[index] = scale;

    return;
}


bool Module_create_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    if (module->scales[index] != NULL)
    {
        Scale_clear(module->scales[index]);
        return true;
    }

    module->scales[index] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (module->scales[index] == NULL)
        return false;

    return true;
}


void Module_remove_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    if (module->scales[index] != NULL)
    {
        del_Scale(module->scales[index]);
        module->scales[index] = NULL;
    }

    return;
}


static void Module_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Module* module = (Module*)device;

    // Reset instruments
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL)
            Device_reset((Device*)ins, dstates);
    }

    // Reset effects
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL)
            Device_reset((Device*)eff, dstates);
    }

    Random_reset(module->random);

    return;
}


static void Module_set_random_seed(Module* module, uint64_t seed)
{
    assert(module != NULL);

    module->random_seed = seed;
    Random_set_seed(module->random, seed);

    return;
}


static bool Module_set_audio_rate(
        const Device* device,
        Device_states* dstates,
        int32_t audio_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(audio_rate > 0);

    const Module* module = (const Module*)device;

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL &&
                !Device_set_audio_rate((Device*)ins, dstates, audio_rate))
            return false;
    }

    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL &&
                !Device_set_audio_rate((Device*)eff, dstates, audio_rate))
            return false;
    }

    return true;
}


static void Module_update_tempo(
        const Device* device,
        Device_states* dstates,
        double tempo)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    const Module* module = (const Module*)device;

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL)
            Device_update_tempo((const Device*)ins, dstates, tempo);
    }

    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL)
            Device_update_tempo((const Device*)eff, dstates, tempo);
    }

    return;
}


static bool Module_set_buffer_size(
        const Device* device,
        Device_states* dstates,
        int32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);

    const Module* module = (const Module*)device;

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL &&
                !Device_set_buffer_size((Device*)ins, dstates, size))
            return false;
    }

    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL &&
                !Device_set_buffer_size((Device*)eff, dstates, size))
            return false;
    }

    return true;
}


void del_Module(Module* module)
{
    if (module == NULL)
        return;

    del_Environment(module->env);
    del_Song_table(module->songs);
    del_Pat_table(module->pats);
    del_Connections(module->connections);
    del_Ins_table(module->insts);
    del_Input_map(module->ins_map);
    del_Effect_table(module->effects);
    del_Track_list(module->track_list);

    for (int i = 0; i < KQT_SONGS_MAX; ++i)
        del_Order_list(module->order_lists[i]);

    for (int i = 0; i < KQT_SCALES_MAX; ++i)
        del_Scale(module->scales[i]);

    del_Random(module->random);
    del_Bind(module->bind);

    Device_deinit(&module->parent);
    memory_free(module);

    return;
}


