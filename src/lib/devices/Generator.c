

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
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
#include <stdio.h>
#include <inttypes.h>

#include <debug/assert.h>
#include <devices/generators/Gen_type.h>
#include <devices/generators/Generator_common.h>
#include <devices/Generator.h>
#include <Filter.h>
#include <memory.h>
#include <pitch_t.h>


static Device_state* Generator_create_state_plain(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Gen_state* gen_state = memory_alloc_item(Gen_state);
    if (gen_state == NULL)
        return NULL;

    Gen_state_init(gen_state, device, audio_rate, audio_buffer_size);

    return &gen_state->parent;
}


Generator* new_Generator(const Instrument_params* ins_params)
{
    assert(ins_params != NULL);

    Generator* gen = memory_alloc_item(Generator);
    if (gen == NULL)
        return NULL;

    if (!Device_init(&gen->parent, true))
    {
        memory_free(gen);
        return NULL;
    }

#if 0
    if (state->error)
        return NULL;

    char type[GEN_TYPE_LENGTH_MAX] = { '\0' };
    read_string(str, type, GEN_TYPE_LENGTH_MAX, state);
    if (state->error)
        return NULL;

    Generator_cons* cons = Gen_type_find_cons(type);
    if (cons == NULL)
    {
        Read_state_set_error(state, "Unsupported Generator type: %s", type);
        return NULL;
    }

    Generator* gen = cons(buffer_size, mix_rate);
    if (gen == NULL)
        return NULL;
#endif

    //fprintf(stderr, "New Generator %p\n", (void*)gen);
    //strcpy(gen->type, type);
    gen->ins_params = ins_params;

    gen->init_vstate = NULL;
    gen->mix = NULL;

    Device_set_state_creator(
            &gen->parent,
            Generator_create_state_plain);

    return gen;
}


bool Generator_init(
        Generator* gen,
        Generator_mix_func mix,
        void (*init_vstate)(const Generator*, const Gen_state*, Voice_state*))
{
    assert(gen != NULL);
    assert(mix != NULL);

    gen->mix = mix;
    gen->init_vstate = init_vstate;

    return true;
}


#if 0
void Generator_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);
    (void)dstates;

    Generator* gen = (Generator*)device;
    Device_params_reset(gen->conf->params);

    return;
}
#endif


#if 0
const char* Generator_get_type(const Generator* gen)
{
    assert(gen != NULL);
    return gen->type;
}
#endif


/**
 * Update voice parameter settings that depend on audio rate and/or tempo.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param freq     The audio rate -- must be positive.
 * \param tempo    The tempo -- must be positive and finite.
 */
static void adjust_relative_lengths(Voice_state* vstate, uint32_t freq, double tempo)
{
    assert(vstate != NULL);
    assert(freq > 0);
    assert(tempo > 0);
    assert(isfinite(tempo));

    if (vstate->freq != freq || vstate->tempo != tempo)
    {
        Slider_set_mix_rate(&vstate->pitch_slider, freq);
        Slider_set_tempo(&vstate->pitch_slider, tempo);
        LFO_set_mix_rate(&vstate->vibrato, freq);
        LFO_set_tempo(&vstate->vibrato, tempo);

        if (vstate->arpeggio)
        {
            vstate->arpeggio_length *= (double)freq / vstate->freq;
            vstate->arpeggio_length *= vstate->tempo / tempo;
            vstate->arpeggio_frames *= (double)freq / vstate->freq;
            vstate->arpeggio_frames *= vstate->tempo / tempo;
        }

        Slider_set_mix_rate(&vstate->force_slider, freq);
        Slider_set_tempo(&vstate->force_slider, tempo);
        LFO_set_mix_rate(&vstate->tremolo, freq);
        LFO_set_tempo(&vstate->tremolo, tempo);

        Slider_set_mix_rate(&vstate->panning_slider, freq);
        Slider_set_tempo(&vstate->panning_slider, tempo);

        Slider_set_mix_rate(&vstate->lowpass_slider, freq);
        Slider_set_tempo(&vstate->lowpass_slider, tempo);
        LFO_set_mix_rate(&vstate->autowah, freq);
        LFO_set_tempo(&vstate->autowah, tempo);

        vstate->freq = freq;
        vstate->tempo = tempo;
    }

    return;
}


void Generator_mix(
        const Generator* gen,
        Device_states* dstates,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo)
{
    assert(gen != NULL);
    assert(gen->mix != NULL);
    assert(dstates != NULL);
    assert(vstate != NULL);
    assert(wbs != NULL);
    assert(freq > 0);
    assert(tempo > 0);

    if (!vstate->active)
        return;

    // Check for voice cut before mixing anything (no need for volume ramping)
    if (!vstate->note_on &&
            (vstate->pos == 0) &&
            (vstate->pos_rem == 0) &&
            !gen->ins_params->env_force_rel_enabled)
    {
        vstate->active = false;
        return;
    }

    if (offset < nframes)
    {
        Gen_state* gen_state = (Gen_state*)Device_states_get_state(
                dstates,
                Device_get_id(&gen->parent));
        Ins_state* ins_state = (Ins_state*)Device_states_get_state(
                dstates,
                gen->ins_params->device_id);

        adjust_relative_lengths(vstate, freq, tempo);

        Generator_common_handle_pitch(gen, vstate, wbs, nframes, offset);

        const int32_t force_stop = Generator_common_handle_force(
                gen, ins_state, vstate, wbs, freq, nframes, offset);

        const bool force_ended = (force_stop < (int32_t)nframes);
        if (force_ended)
            nframes = force_stop;

        gen->mix(gen, gen_state, ins_state, vstate, wbs, nframes, offset, freq, tempo);

        if (force_ended)
            vstate->active = false;
    }

    return;
}


void del_Generator(Generator* gen)
{
    if (gen == NULL)
        return;

    Device_deinit(&gen->parent);
    memory_free(gen);

    return;
}


