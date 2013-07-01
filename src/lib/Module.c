

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
static void Module_reset(Device* device);


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
 * \param mix_rate   The mixing frequency -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Module_set_mix_rate(Device* device, uint32_t mix_rate);


/**
 * Sets the buffer size of the Module.
 *
 * This function sets the buffer size for all the Instruments and Effects.
 *
 * \param device   The Module Device -- must not be \c NULL.
 * \param size     The new buffer size -- must be > \c 0 and
 *                 <= \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
static bool Module_set_buffer_size(Device* device, uint32_t size);


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
static bool Module_sync(Device* device);


Module* new_Module(uint32_t buf_size)
{
    assert(buf_size > 0);
    assert(buf_size <= KQT_BUFFER_SIZE_MAX);
    Module* module = memory_alloc_item(Module);
    if (module == NULL)
    {
        return NULL;
    }
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
    module->subsongs = NULL;
    module->pats = NULL;
    module->insts = NULL;
    module->effects = NULL;
    module->connections = NULL;
    module->play_state = NULL;
    module->event_handler = NULL;
    module->skip_state = NULL;
    module->skip_handler = NULL;
    module->random = NULL;
    module->env = NULL;
    module->bind = NULL;
    module->album_is_existent = false;
    module->track_list = NULL;
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        module->channels[i] = NULL;
    }
    for (int i = 0; i < KQT_SONGS_MAX; ++i)
    {
        module->order_lists[i] = NULL;
    }
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        module->scales[i] = NULL;
    }

    // Create fields
    module->random = new_Random();
    module->subsongs = new_Subsong_table();
    module->pats = new_Pat_table(KQT_PATTERNS_MAX);
    module->insts = new_Ins_table(KQT_INSTRUMENTS_MAX);
    module->effects = new_Effect_table(KQT_EFFECTS_MAX);
    if (module->random == NULL       ||
            module->subsongs == NULL ||
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
    module->play_state = new_Playdata(module->insts,
                                    module->env,
                                    module->random);
    if (module->play_state == NULL)
    {
        del_Module(module);
        return NULL;
    }
    module->play_state->subsongs = Module_get_subsongs(module);
    module->play_state->order_lists = module->order_lists;
    module->play_state->scales = module->scales;
    module->play_state->active_scale = &module->play_state->scales[0];
    module->skip_state = new_Playdata_silent(module->env, 1000000000);
    if (module->skip_state == NULL)
    {
        del_Module(module);
        return NULL;
    }
    module->skip_state->subsongs = Module_get_subsongs(module);
    module->skip_state->order_lists = module->order_lists;

    if (!Device_init_buffer(&module->parent, DEVICE_PORT_TYPE_RECEIVE, 0))
    {
        del_Module(module);
        return NULL;
    }
    Read_state* conn_state = READ_STATE_AUTO;
    module->connections = new_Connections_from_string(NULL, false,
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
    if (!Connections_prepare(module->connections))
    {
        del_Module(module);
        return NULL;
    }

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        module->channels[i] = new_Channel(module->insts, i,
                                        module->play_state->voice_pool,
                                        module->env,
                                        &module->play_state->tempo,
                                        &module->play_state->freq);
        if (module->channels[i] == NULL)
        {
            del_Module(module);
            return NULL;
        }
    }
    Channel_state* ch_states[KQT_COLUMNS_MAX] = { NULL };
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        ch_states[i] = &module->channels[i]->cur_state;
    }

    module->event_handler = new_Event_handler(NULL, module->play_state,
                                            ch_states,
                                            module->insts,
                                            module->effects);
    module->skip_handler = new_Event_handler(NULL, module->skip_state,
                                           ch_states,
                                           module->insts,
                                           module->effects);
    if (module->event_handler == NULL || module->skip_handler == NULL)
    {
        del_Module(module);
        return NULL;
    }
    Read_state* state = READ_STATE_AUTO;
    Bind* bind = new_Bind(NULL, Event_handler_get_names(module->event_handler),
                          state);
    if (bind == NULL || !Module_set_bind(module, bind))
    {
        assert(!state->error);
        del_Bind(bind);
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
    module->mix_vol_dB = SONG_DEFAULT_MIX_VOL;
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
    {
        return false;
    }
    double mix_vol = SONG_DEFAULT_MIX_VOL;

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
                if (string_eq(key, "mix_vol"))
                {
                    str = read_double(str, &mix_vol, state);
                    if (state->error)
                    {
                        return false;
                    }
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

    module->mix_vol_dB = mix_vol;
    module->mix_vol = exp2(module->mix_vol_dB / 6);
    return true;
}


bool Module_parse_random_seed(Module* module, char* str, Read_state* state)
{
    assert(module != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    int64_t seed = 0;
    if (str != NULL)
    {
        str = read_int(str, &seed, state);
        if (state->error)
        {
            return false;
        }
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

    if (!Subsong_table_get_existent(module->subsongs, song))
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


uint32_t Module_mix(Module* module, uint32_t nframes, Event_handler* eh)
{
    assert(module != NULL);
    assert(eh != NULL);
    Playdata* play = Event_handler_get_global_state(eh);
    if (play->mode == STOP)
    {
        return 0;
    }

    if (nframes > Device_get_buffer_size((Device*)module) && !play->silent)
    {
        nframes = Device_get_buffer_size((Device*)module);
    }
    if (!play->silent)
    {
        if (module->connections != NULL)
        {
            Connections_clear_buffers(module->connections, 0, nframes);
        }
        Voice_pool_prepare(play->voice_pool);
    }
    uint32_t mixed = 0;
    while (mixed < nframes && play->mode)
    {
        // FIXME: setting the tempo on both states breaks initial tempo
        //        if it is set after calculating duration
        Playdata* states[2] = { module->play_state, module->skip_state, };
        for (int i = 0; i < 2; ++i)
        {
            Playdata* state = states[i];
            if (isnan(state->tempo))
            {
                state->tempo = 120;
                const uint16_t track_index = state->track;
                const Track_list* tl = state->track_list;
                if (tl != NULL && track_index < Track_list_get_len(tl))
                {
                    int16_t module_index = Track_list_get_song_index(
                            tl, state->track);
                    const bool existent = Subsong_table_get_existent(
                            play->subsongs,
                            module_index);
                    Subsong* ss = Subsong_table_get(
                            play->subsongs,
                            module_index);
                    if (existent && ss != NULL)
                        state->tempo = Subsong_get_tempo(ss);
                }
            }
        }

        Pattern* pat = NULL;
        if (play->mode >= PLAY_SUBSONG)
        {
            // XXX: editing the track list while playing may work unexpectedly
            const uint16_t track_index = play->track;
            const Track_list* tl = play->track_list;
            if (tl != NULL && track_index < Track_list_get_len(tl))
            {
                const int16_t module_index = Track_list_get_song_index(
                        tl, track_index);
                const bool existent = Subsong_table_get_existent(
                        module->subsongs,
                        module_index);
                const Order_list* ol = module->order_lists[module_index];
                if (existent && ol != NULL && play->system < Order_list_get_len(ol))
                {
                    Pat_inst_ref* ref = Order_list_get_pat_inst_ref(
                            ol, play->system);
                    assert(ref != NULL);
                    play->piref = *ref;
                }
            }
#if 0
            Subsong* ss = Subsong_table_get(module->subsongs, play->subsong);
            if (ss != NULL)
            {
                play->pattern = Subsong_get(ss, play->section);
            }
#endif
            if (play->piref.pat >= 0)
            {
                pat = Pat_table_get(module->pats, play->piref.pat);
                if (!Pat_table_get_existent(module->pats, play->piref.pat))
                    pat = NULL;
                if (pat != NULL &&
                        !Pattern_get_inst_existent(pat, play->piref.inst))
                    pat = NULL;
            }
        }
        else if (play->mode == PLAY_PATTERN && play->piref.pat >= 0)
        {
            pat = Pat_table_get(module->pats, play->piref.pat);
            if (!Pat_table_get_existent(module->pats, play->piref.pat))
                pat = NULL;
            if (pat != NULL &&
                    !Pattern_get_inst_existent(pat, play->piref.inst))
                pat = NULL;
        }
        if (pat == NULL && !play->parent.pause)
        {
            if (play->mode == PLAY_PATTERN)
            {
                play->mode = STOP;
                break;
            }
            else if (play->mode == PLAY_SUBSONG)
            {
                if (play->infinite && play->play_frames + mixed > 0)
                {
                    Playdata_set_track(play, play->orig_track, false);
                    continue;
                }
                else
                {
                    play->mode = STOP;
                    break;
                }
            }
            assert(play->mode == PLAY_SONG);
            if (play->track >= KQT_TRACKS_MAX - 1)
            {
                Playdata_set_track(play, 0, !play->infinite);
                if (play->infinite && play->play_frames + mixed > 0)
                {
                    continue;
                }
                else
                {
                    play->mode = STOP;
                    break;
                }
            }
            Playdata_set_track(play, play->track + 1,
                    !play->infinite);
            continue;
        }
        mixed += Pattern_mix(pat, nframes, mixed, eh, module->channels,
                             module->connections);
    }
    Audio_buffer* buffer = NULL;
    if (module->connections != NULL)
    {
        Device* master = Device_node_get_device(
                                 Connections_get_master(module->connections));
        buffer = Device_get_buffer(master,
                         DEVICE_PORT_TYPE_RECEIVE, 0);
    }
    if (!play->silent && buffer != NULL)
    {
        kqt_frame* bufs[] =
        {
            Audio_buffer_get_buffer(buffer, 0),
            Audio_buffer_get_buffer(buffer, 1),
        };
        assert(bufs[0] != NULL);
        assert(bufs[1] != NULL);
        for (int i = 0; i < 2; ++i)
        {
            for (uint32_t k = 0; k < mixed; ++k)
            {
                bufs[i][k] *= module->mix_vol;
                if (bufs[i][k] < play->min_amps[i])
                {
                    play->min_amps[i] = bufs[i][k];
                }
                if (bufs[i][k] > play->max_amps[i])
                {
                    play->max_amps[i] = bufs[i][k];
                }
                if (fabs(bufs[i][k]) > 1)
                {
                    ++play->clipped[i];
                }
            }
        }
    }
    if (!play->parent.pause)
    {
        play->play_frames += mixed;
    }
#if 0
    fprintf(stderr, "%3d  \r", play->active_voices);
    play->active_voices = 0;
#endif
    return mixed;
}


uint64_t Module_skip(Module* module, Event_handler* eh, uint64_t amount)
{
    assert(module != NULL);
    assert(eh != NULL);
    Playdata* play = Event_handler_get_global_state(eh);
    bool orig_silent = play->silent;
    play->silent = true;
    uint64_t mixed = 0;
    while (mixed < amount)
    {
        uint64_t max_mix = amount - mixed;
        uint64_t nframes = MIN(max_mix, play->freq);
        uint32_t inc = Module_mix(module, nframes, eh);
        mixed += inc;
        if (inc < Device_get_buffer_size((Device*)module))
        {
            break;
        }
    }
    play->silent = orig_silent;
    return mixed;
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


#if 0
void Module_set_subsong(Module* module, uint16_t num)
{
    assert(module != NULL);
    assert(num < KQT_SONGS_MAX);
    module->init_subsong = num;
    return;
}
#endif


#if 0
uint16_t Module_get_subsong(Module* module)
{
    assert(module != NULL);
    return module->init_subsong;
}
#endif


Subsong_table* Module_get_subsongs(const Module* module)
{
    assert(module != NULL);
    return module->subsongs;
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
    assert(module->bind == module->play_state->bind);
    assert(module->bind == module->skip_state->bind);
    Event_cache* caches[KQT_COLUMNS_MAX] = { NULL };
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        caches[i] = Bind_create_cache(bind);
        if (caches[i] == NULL)
        {
            for (int k = i - 1; k >= 0; --k)
            {
                del_Event_cache(caches[k]);
            }
            return false;
        }
    }
    del_Bind(module->bind);
    module->bind = module->play_state->bind = module->skip_state->bind = bind;
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_set_event_cache(module->channels[i], caches[i]);
    }
    return true;
}


Scale** Module_get_scales(Module* module)
{
    assert(module != NULL);
    return module->play_state->scales;
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
    {
        del_Scale(module->scales[index]);
    }
    module->scales[index] = scale;
    return;
}


Scale*** Module_get_active_scale(Module* module)
{
    assert(module != NULL);
    return &module->play_state->active_scale;
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
    {
        return false;
    }
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


static void Module_reset(Device* device)
{
    assert(device != NULL);
    Module* module = (Module*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL)
        {
            Device_reset((Device*)ins);
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL)
        {
            Device_reset((Device*)eff);
        }
    }
    Event_handler_clear_buffers(module->event_handler);
    Playdata_reset(module->play_state);
    Playdata_reset(module->skip_state);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_reset(module->channels[i]);
    }
    Random_reset(module->random);
    return;
}


static void Module_set_random_seed(Module* module, uint64_t seed)
{
    assert(module != NULL);
    module->random_seed = seed;
    Random_set_seed(module->random, seed);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_set_random_seed(module->channels[i], seed);
    }
    return;
}


static bool Module_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    Module* module = (Module*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL && !Device_set_mix_rate((Device*)ins, mix_rate))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL && !Device_set_mix_rate((Device*)eff, mix_rate))
        {
            return false;
        }
    }
    return true;
}


static bool Module_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    Module* module = (Module*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL && !Device_set_buffer_size((Device*)ins, size))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL && !Device_set_buffer_size((Device*)eff, size))
        {
            return false;
        }
    }
    return true;
}


static bool Module_sync(Device* device)
{
    assert(device != NULL);
    Module* module = (Module*)device;
    for (int i = 0; i < KQT_INSTRUMENTS_MAX; ++i)
    {
        Instrument* ins = Ins_table_get(module->insts, i);
        if (ins != NULL && !Device_sync((Device*)ins))
        {
            return false;
        }
    }
    for (int i = 0; i < KQT_EFFECTS_MAX; ++i)
    {
        Effect* eff = Effect_table_get(module->effects, i);
        if (eff != NULL && !Device_sync((Device*)eff))
        {
            return false;
        }
    }
    return true;
}


void del_Module(Module* module)
{
    if (module == NULL)
    {
        return;
    }
    del_Environment(module->env);
    del_Subsong_table(module->subsongs);
    del_Pat_table(module->pats);
    del_Connections(module->connections);
    del_Ins_table(module->insts);
    del_Effect_table(module->effects);
    del_Track_list(module->track_list);
    for (int i = 0; i < KQT_SONGS_MAX; ++i)
    {
        del_Order_list(module->order_lists[i]);
    }
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
    {
        del_Scale(module->scales[i]);
    }
    del_Playdata(module->play_state);
    del_Playdata(module->skip_state);
    for (int i = 0; i < KQT_COLUMNS_MAX && module->channels[i] != NULL; ++i)
    {
        del_Channel(module->channels[i]);
    }
    del_Event_handler(module->event_handler);
    del_Event_handler(module->skip_handler);
    del_Random(module->random);
    del_Bind(module->bind);
    Device_uninit(&module->parent);
    memory_free(module);
    return;
}


