

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


/**
 * Sets the mixing rate of the Module.
 *
 * This function sets the mixing rate for all the Instruments and Effects.
 *
 * \param device     The Module Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mix_rate   The mixing frequency -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Module_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate);


/**
 * Sets the buffer size of the Module.
 *
 * This function sets the buffer size for all the Instruments and Effects.
 *
 * \param device    The Module Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param size      The new buffer size -- must be > \c 0 and
 *                  <= \c KQT_AUDIO_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Module_set_buffer_size(
        Device* device,
        Device_states* dstates,
        uint32_t size);


/**
 * Synchronises the Module.
 *
 * This function synchronises all the Devices the Module contains. It should be
 * called after loading a Kunquat composition.
 *
 * \param device   The Module Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Module_sync(Device* device, Device_states* dstates);


Module* new_Module(uint32_t buf_size)
{
    assert(buf_size > 0);
    assert(buf_size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    Module* module = memory_alloc_item(Module);
    if (module == NULL)
        return NULL;

    if (!Device_init(&module->parent, buf_size, 48000))
    {
        memory_free(module);
        return NULL;
    }

    Device_set_existent(&module->parent, true);
    Device_set_reset(&module->parent, Module_reset);
    Device_set_mix_rate_changer(&module->parent, Module_set_mix_rate);
    Device_set_buffer_size_changer(&module->parent, Module_set_buffer_size);
    Device_set_sync(&module->parent, Module_sync);
    Device_register_port(&module->parent, DEVICE_PORT_TYPE_RECEIVE, 0);

    // Clear fields
    module->songs = NULL;
    module->pats = NULL;
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

    Read_state* conn_state = READ_STATE_AUTO;
    module->connections = new_Connections_from_string(
            NULL,
            false,
            module->insts,
            module->effects,
            NULL,
            &module->parent,
            conn_state);
    if (module->connections == NULL)
    {
        assert(!conn_state->error);
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


bool Module_parse_composition(Module* module, char* str, Read_state* state)
{
    assert(module != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    double mix_vol = MODULE_DEFAULT_MIX_VOL;

    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
            return false;

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
                    return false;

                if (string_eq(key, "mix_vol"))
                {
                    str = read_double(str, &mix_vol, state);
                    if (state->error)
                        return false;

                    if (!isfinite(mix_vol) && mix_vol != -INFINITY)
                    {
                        Read_state_set_error(state,
                                 "Invalid mixing volume: %f", module->mix_vol_dB);
                        return false;
                    }
                }
                else
                {
                    Read_state_set_error(state,
                             "Unrecognised key in composition info: %s", key);
                    return false;
                }

                if (state->error)
                    return false;

                check_next(str, state, expect_key);
            }

            str = read_const_char(str, '}', state);
            if (state->error)
                return false;
        }
    }

    module->mix_vol_dB = mix_vol;
    module->mix_vol = exp2(module->mix_vol_dB / 6);

    return true;
}


bool Module_parse_random_seed(Module* module, char* str, Read_state* state)
{
    assert(module != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    int64_t seed = 0;
    if (str != NULL)
    {
        str = read_int(str, &seed, state);
        if (state->error)
            return false;

        if (seed < 0)
        {
            Read_state_set_error(state, "Random seed must be positive");
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


bool Module_set_bind(Module* module, Bind* bind)
{
    assert(module != NULL);
    assert(bind != NULL);

#if 0
    Event_cache* caches[KQT_COLUMNS_MAX] = { NULL };
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        caches[i] = Bind_create_cache(bind);
        if (caches[i] == NULL)
        {
            for (int k = i - 1; k >= 0; --k)
                del_Event_cache(caches[k]);
            return false;
        }
    }
    del_Bind(module->bind);
    module->bind = module->play_state->bind = module->skip_state->bind = bind;
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
        Channel_set_event_cache(module->channels[i], caches[i]);
#endif

    return true;
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


static bool Module_set_mix_rate(
        Device* device,
        Device_states* dstates,
        uint32_t mix_rate)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(mix_rate > 0);

    Module* module = (Module*)device;

    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL &&
                !Device_set_mix_rate((Device*)ins, dstates, mix_rate))
            return false;
    }

    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL &&
                !Device_set_mix_rate((Device*)eff, dstates, mix_rate))
            return false;
    }

    return true;
}


static bool Module_set_buffer_size(
        Device* device,
        Device_states* dstates,
        uint32_t size)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Module* module = (Module*)device;

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


static bool Module_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Module* module = (Module*)device;

    // Sync instruments
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL && !Device_sync((Device*)ins, dstates))
            return false;
    }

    // Sync effects
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL && !Device_sync((Device*)eff, dstates))
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


