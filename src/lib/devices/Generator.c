

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

    if (offset >= nframes)
        return;

    // Get states
    Gen_state* gen_state = (Gen_state*)Device_states_get_state(
            dstates,
            Device_get_id(&gen->parent));
    Ins_state* ins_state = (Ins_state*)Device_states_get_state(
            dstates,
            gen->ins_params->device_id);

    // Get audio output buffers
    Audio_buffer* audio_buffer = Device_state_get_audio_buffer(
            &gen_state->parent, DEVICE_PORT_TYPE_SEND, 0);
    if (audio_buffer == NULL)
        return;
    kqt_frame* out_l = Audio_buffer_get_buffer(audio_buffer, 0);
    kqt_frame* out_r = Audio_buffer_get_buffer(audio_buffer, 1);

    // Process common parameters required by implementations
    bool deactivate_after_processing = false;
    int32_t process_stop = nframes;

    adjust_relative_lengths(vstate, freq, tempo);

    Generator_common_handle_pitch(gen, vstate, wbs, process_stop, offset);

    const int32_t force_stop = Generator_common_handle_force(
            gen, ins_state, vstate, wbs, freq, process_stop, offset);

    const bool force_ended = (force_stop < process_stop);
    if (force_ended)
    {
        deactivate_after_processing = true;
        assert(force_stop <= process_stop);
        process_stop = force_stop;
    }

    const uint64_t old_pos = vstate->pos;
    const double old_pos_rem = vstate->pos_rem;

    // Call the implementation
    const int32_t impl_render_stop = gen->mix(
            gen, gen_state, ins_state, vstate, wbs, process_stop, offset, freq, tempo);
    if (!vstate->active) // FIXME: communicate end of rendering in a cleaner way
    {
        vstate->active = true;
        deactivate_after_processing = true;
        assert(impl_render_stop <= process_stop);
        process_stop = impl_render_stop;
    }

    // XXX: Hack to make post-processing work correctly below, fix properly!
    const uint64_t new_pos = vstate->pos;
    const double new_pos_rem = vstate->pos_rem;
    vstate->pos = old_pos;
    vstate->pos_rem = old_pos_rem;

    // Apply common parameters to generated signal
    const int32_t ramp_release_stop = Generator_common_ramp_release(
            gen, ins_state, vstate, wbs, 2, freq, process_stop, offset);
    const bool ramp_release_ended = (vstate->ramp_release >= 1);
    if (ramp_release_ended)
    {
        deactivate_after_processing = true;
        assert(ramp_release_stop <= process_stop);
        process_stop = ramp_release_stop;
    }

    Generator_common_handle_filter(gen, vstate, wbs, 2, freq, process_stop, offset);
    Generator_common_handle_panning(gen, vstate, wbs, process_stop, offset);

    vstate->pos = new_pos;
    vstate->pos_rem = new_pos_rem;

    // Mix rendered audio
    {
        const Work_buffer* wb_audio_l = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_L);
        const Work_buffer* wb_audio_r = Work_buffers_get_buffer(
                wbs, WORK_BUFFER_AUDIO_R);
        float* audio_l = Work_buffer_get_contents_mut(wb_audio_l);
        float* audio_r = Work_buffer_get_contents_mut(wb_audio_r);

        for (int32_t i = offset; i < process_stop; ++i)
            out_l[i] += audio_l[i];
        for (int32_t i = offset; i < process_stop; ++i)
            out_r[i] += audio_r[i];
    }

    if (deactivate_after_processing)
        vstate->active = false;

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


