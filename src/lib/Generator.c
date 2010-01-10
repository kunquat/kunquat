

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
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <Generator.h>
#include <Generator_sine.h>
#include <Generator_sawtooth.h>
#include <Generator_triangle.h>
#include <Generator_square.h>
#include <Generator_square303.h>
#include <Generator_pcm.h>
#include <File_base.h>
#include <Filter.h>
#include <Event_ins.h>

#include <xmemory.h>


Generator* new_Generator(Gen_type type, Instrument_params* ins_params)
{
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    assert(ins_params != NULL);
    Generator* (*cons[])(Instrument_params*) =
    {
        [GEN_TYPE_SINE] = new_Generator_sine,
        [GEN_TYPE_SAWTOOTH] = new_Generator_sawtooth,
        [GEN_TYPE_TRIANGLE] = new_Generator_triangle,
        [GEN_TYPE_SQUARE] = new_Generator_square,
        [GEN_TYPE_SQUARE303] = new_Generator_square303,
        [GEN_TYPE_PCM] = new_Generator_pcm,
    };
    assert(cons[type] != NULL);
    Generator* gen = cons[type](ins_params);
//    if (type == GEN_TYPE_PCM) fprintf(stderr, "returning new pcm %p\n", (void*)gen);
    return gen;
}


bool Generator_init(Generator* gen)
{
    assert(gen != NULL);
    gen->enabled = GENERATOR_DEFAULT_ENABLED;
    gen->volume_dB = GENERATOR_DEFAULT_VOLUME;
    gen->volume = exp2(gen->volume_dB / 6);
    gen->parse = NULL;
    return true;
}


void Generator_uninit(Generator* gen)
{
    assert(gen != NULL);
    (void)gen;
    return;
}


void Generator_copy_general(Generator* dest, Generator* src)
{
    assert(dest != NULL);
    assert(src != NULL);
    dest->enabled = src->enabled;
    dest->volume_dB = src->volume_dB;
    dest->volume = src->volume;
    return;
}


bool Generator_parse_general(Generator* gen, char* str, Read_state* state)
{
    assert(gen != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    bool enabled = false;
    double volume = 0;
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
            char key[128] = { '\0' };
            bool expect_key = true;
            while (expect_key)
            {
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (strcmp(key, "enabled") == 0)
                {
                    str = read_bool(str, &enabled, state);
                }
                else if (strcmp(key, "volume") == 0)
                {
                    str = read_double(str, &volume, state);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unsupported key in Generator info: %s", key);
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
    gen->enabled = enabled;
    gen->volume_dB = volume;
    gen->volume = exp2(gen->volume_dB / 6);
    return true;
}


Gen_type Generator_type_parse(char* str, Read_state* state)
{
    assert(state != NULL);
    if (state->error)
    {
        return GEN_TYPE_LAST;
    }
    if (str == NULL)
    {
        return GEN_TYPE_NONE;
    }
    static const char* map[GEN_TYPE_LAST] =
    {
        [GEN_TYPE_SINE] = "sine",
        [GEN_TYPE_TRIANGLE] = "triangle",
        [GEN_TYPE_SQUARE] = "square",
        [GEN_TYPE_SQUARE303] = "square303",
        [GEN_TYPE_SAWTOOTH] = "sawtooth",
        [GEN_TYPE_PCM] = "pcm",
    };
    char desc[128] = { '\0' };
    str = read_string(str, desc, 128, state);
    if (state->error)
    {
        return GEN_TYPE_LAST;
    }
    for (Gen_type i = GEN_TYPE_NONE; i < GEN_TYPE_LAST; ++i)
    {
        if (map[i] != NULL && strcmp(map[i], desc) == 0)
        {
            return i;
        }
    }
    Read_state_set_error(state, "Unsupported Generator type: %s", desc);
    return GEN_TYPE_LAST;
}


bool Generator_type_has_subkey(Gen_type type, const char* subkey)
{
    assert(type > GEN_TYPE_NONE);
    assert(type < GEN_TYPE_LAST);
    if (subkey == NULL)
    {
        return false;
    }
    static bool (*map[GEN_TYPE_LAST])(const char*) =
    {
        [GEN_TYPE_PCM] = Generator_pcm_has_subkey,
        [GEN_TYPE_SQUARE] = Generator_square_has_subkey,
    };
    if (map[type] == NULL)
    {
        return false;
    }
    return map[type](subkey);
}


bool Generator_parse(Generator* gen,
                     const char* subkey,
                     void* data,
                     long length,
                     Read_state* state)
{
    assert(gen != NULL);
    assert(subkey != NULL);
    assert(Generator_type_has_subkey(Generator_get_type(gen), subkey));
    assert(data != NULL || length == 0);
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    assert(gen->parse != NULL);
    return gen->parse(gen, subkey, data, length, state);
}


Gen_type Generator_get_type(Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}


void Generator_process_note(Generator* gen,
                            Voice_state* state,
                            int note,
                            int mod,
                            int octave)
{
    assert(gen != NULL);
    assert(state != NULL);
    assert(note >= 0);
    assert(note < KQT_SCALE_NOTES);
    assert(mod < KQT_SCALE_NOTE_MODS);
    assert(octave >= KQT_SCALE_OCTAVE_FIRST);
    assert(octave <= KQT_SCALE_OCTAVE_LAST);
    if (gen->ins_params->scale == NULL ||
            *gen->ins_params->scale == NULL ||
            **gen->ins_params->scale == NULL)
    {
        return;
    }
    pitch_t pitch = Scale_get_pitch(**gen->ins_params->scale, note, mod, octave);
    if (pitch > 0)
    {
        state->pitch = pitch;
    }
    return;
}


#if 0
bool Generator_add_event(Generator* gen, Event* event, uint32_t pos)
{
    assert(gen != NULL);
    assert(event != NULL);
    return Event_queue_ins(gen->events, event, pos);
}
#endif


void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq,
                   double tempo)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    uint32_t mixed = offset;
    while (mixed < nframes)
    {
        kqt_frame** bufs = gen->ins_params->bufs;
        if ((state->filter_update && state->filter_xfade_pos >= 1)
                || freq != state->freq)
        {
            state->filter_xfade_state_used = state->filter_state_used;
            state->filter_xfade_pos = 0;
            state->filter_xfade_update = 200.0 / freq;
            if (state->actual_filter < freq / 2)
            {
                int new_state = (state->filter_state_used + 1) % 2;
                bilinear_butterworth_lowpass_filter_create(FILTER_ORDER,
                        state->actual_filter / freq,
                        state->filter_resonance,
                        state->filter_state[new_state].coeffs1,
                        state->filter_state[new_state].coeffs2);
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    for (int k = 0; k < FILTER_ORDER; ++k)
                    {
                        state->filter_state[new_state].history1[i][k] = 0;
                        state->filter_state[new_state].history2[i][k] = 0;
                    }
                }
                state->filter_state_used = new_state;
            }
            else
            {
                if (state->filter_state_used == -1)
                {
                    state->filter_xfade_pos = 1;
                }
                state->filter_state_used = -1;
            }
            state->effective_filter = state->actual_filter;
            state->effective_resonance = state->filter_resonance;
            state->filter_update = false;
        }

        uint32_t mix_until = nframes;
#if 0
        Event* ins_event = NULL;
        uint32_t ins_event_pos = 0;
        if (Event_queue_peek(gen->events, 0, &ins_event, &ins_event_pos))
        {
            if (ins_event_pos < mix_until)
            {
                mix_until = ins_event_pos;
            }
            else
            {
                ins_event = NULL;
            }
        }
#endif

        if (state->filter_state_used > -1 || state->filter_xfade_state_used > -1)
        {
            bufs = gen->ins_params->vbufs;
            if (state->filter_xfade_pos < 1 && mix_until - offset >
                    (1 - state->filter_xfade_pos) / state->filter_xfade_update)
            {
                mix_until = offset +
                        ceil((1 - state->filter_xfade_pos) / state->filter_xfade_update);
            }
        }

        mixed = gen->mix(gen, state, mix_until, mixed, freq, tempo,
                         gen->ins_params->buf_count,
                         bufs);

#if 0
        if (ins_event != NULL && mixed == mix_until)
        {
            fprintf(stderr, "Instrument event at %d! \n", (int)ins_event_pos);
            Event_queue_get(gen->events, &ins_event, &ins_event_pos);
            assert(ins_event_pos == mix_until);
            assert(ins_event != NULL);
            assert(EVENT_IS_INS(Event_get_type(ins_event)));
            Event_ins_process((Event_ins*)ins_event, gen->ins_params);
        }
#endif

        if (bufs == gen->ins_params->vbufs)
        {
            assert(state->filter_state_used != state->filter_xfade_state_used);
            kqt_frame** in_buf = gen->ins_params->vbufs;
            if (state->filter_state_used > -1)
            {
                in_buf = gen->ins_params->vbufs2;
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    iir_filter_df1(FILTER_ORDER, FILTER_ORDER,
                                   state->filter_state[state->filter_state_used].coeffs1,
                                   state->filter_state[state->filter_state_used].coeffs2,
                                   state->filter_state[state->filter_state_used].history1[i],
                                   state->filter_state[state->filter_state_used].history2[i],
                                   mixed - offset,
                                   gen->ins_params->vbufs[i] + offset,
                                   gen->ins_params->vbufs2[i] + offset);
                }
            }
            double vol = state->filter_xfade_pos;
            for (uint32_t k = offset; k < mixed; ++k)
            {
                if (vol > 1)
                {
                    vol = 1;
                }
                for (int i = 0; i < gen->ins_params->buf_count; ++i)
                {
                    gen->ins_params->bufs[i][k] += in_buf[i][k] * vol;
                }
                vol += state->filter_xfade_update;
            }
            if (state->filter_xfade_pos < 1)
            {
                kqt_frame** fade_buf = gen->ins_params->vbufs;
                if (state->filter_xfade_state_used > -1)
                {
                    fade_buf = gen->ins_params->vbufs2;
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        for (uint32_t k = 0; k < nframes; ++k)
                        {
                            gen->ins_params->vbufs2[i][k] = 0;
                        }
                    }
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        iir_filter_df1(FILTER_ORDER, FILTER_ORDER,
                                state->filter_state[state->filter_xfade_state_used].coeffs1,
                                state->filter_state[state->filter_xfade_state_used].coeffs2,
                                state->filter_state[state->filter_xfade_state_used].history1[i],
                                state->filter_state[state->filter_xfade_state_used].history2[i],
                                mixed - offset,
                                gen->ins_params->vbufs[i] + offset,
                                gen->ins_params->vbufs2[i] + offset);
                    }
                }
                double vol = 1 - state->filter_xfade_pos;
                for (uint32_t k = offset; k < mixed; ++k)
                {
                    if (vol <= 0)
                    {
                        break;
                    }
                    for (int i = 0; i < gen->ins_params->buf_count; ++i)
                    {
                        gen->ins_params->bufs[i][k] += fade_buf[i][k] * vol;
                    }
                    vol -= state->filter_xfade_update;
                }
            }
            for (int i = 0; i < gen->ins_params->buf_count; ++i)
            {
                for (uint32_t k = 0; k < nframes; ++k)
                {
                    gen->ins_params->vbufs[i][k] = 0;
                    gen->ins_params->vbufs2[i][k] = 0;
                }
            }
            state->filter_xfade_pos += state->filter_xfade_update * (mixed - offset);
        }
        offset = mixed;
        if (!state->active)
        {
            break;
        }
    }
    return;
}


void del_Generator(Generator* gen)
{
    assert(gen != NULL);
    assert(gen->destroy != NULL);
    gen->destroy(gen);
    return;
}


