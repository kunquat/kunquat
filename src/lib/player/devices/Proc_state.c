

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Proc_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <mathnum/Tstamp.h>
#include <memory.h>
#include <player/devices/Device_state.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static bool Proc_state_set_audio_buffer_size(Device_state* dstate, int32_t new_size);

static void Proc_state_deinit(Proc_state* proc_state);


static Device_state_set_audio_rate_func Proc_state_set_audio_rate;

static Device_state_set_tempo_func Proc_state_set_tempo;

static Device_state_reset_func Proc_state_reset;

static Device_state_render_mixed_func Proc_state_render_mixed;

static Device_state_fire_event_func Proc_state_fire_event;

static Device_state_destroy_func del_Proc_state;


bool Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    rassert(proc_state != NULL);
    rassert(device != NULL);
    rassert(audio_rate > 0);
    rassert(audio_buffer_size >= 0);

    proc_state->destroy = NULL;
    proc_state->set_audio_rate = NULL;
    proc_state->set_audio_buffer_size = NULL;
    proc_state->set_tempo = NULL;
    proc_state->reset = NULL;
    proc_state->render_mixed = NULL;

    proc_state->clear_history = NULL;
    proc_state->fire_dev_event = NULL;

    if (!Device_state_init(&proc_state->parent, device, audio_rate, audio_buffer_size))
        return false;

    proc_state->parent.set_audio_rate = Proc_state_set_audio_rate;
    proc_state->parent.set_audio_buffer_size = Proc_state_set_audio_buffer_size;
    proc_state->parent.set_tempo = Proc_state_set_tempo;
    proc_state->parent.reset = Proc_state_reset;
    proc_state->parent.render_mixed = Proc_state_render_mixed;
    proc_state->parent.fire_dev_event = Proc_state_fire_event;
    proc_state->parent.destroy = del_Proc_state;

    return true;
}


bool Proc_state_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    rassert(dstate != NULL);
    rassert(audio_rate > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->set_audio_rate != NULL)
        return proc_state->set_audio_rate(dstate, audio_rate);

    return true;
}


void Proc_state_set_tempo(Device_state* dstate, double tempo)
{
    rassert(dstate != NULL);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->set_tempo != NULL)
        proc_state->set_tempo(dstate, tempo);

    return;
}


void Proc_state_reset(Device_state* dstate)
{
    rassert(dstate != NULL);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->reset != NULL)
        proc_state->reset(dstate);

    return;
}


void Proc_state_render_mixed(
        Device_state* dstate,
        Device_thread_state* ts,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(dstate != NULL);
    rassert(ts != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(isfinite(tempo));
    rassert(tempo > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->render_mixed != NULL)
        proc_state->render_mixed(dstate, ts, wbs, frame_count, tempo);

    return;
}


void Proc_state_clear_history(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    if (proc_state->clear_history != NULL)
        proc_state->clear_history(proc_state);

    return;
}


static bool Proc_state_set_audio_buffer_size(Device_state* dstate, int32_t new_size)
{
    rassert(dstate != NULL);
    rassert(new_size >= 0);

    Proc_state* proc_state = (Proc_state*)dstate;

    if (proc_state->set_audio_buffer_size != NULL)
        return proc_state->set_audio_buffer_size(dstate, new_size);

    return true;
}


static void Proc_state_deinit(Proc_state* proc_state)
{
    rassert(proc_state != NULL);

    return;
}


static void Proc_state_fire_event(
        Device_state* dstate, const char* event_name, const Value* arg, Random* rand)
{
    rassert(dstate != NULL);
    rassert(event_name != NULL);
    rassert(arg != NULL);
    rassert(rand != NULL);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->fire_dev_event != NULL)
        proc_state->fire_dev_event(dstate, event_name, arg);

    return;
}


static void del_Proc_state(Device_state* dstate)
{
    if (dstate == NULL)
        return;

    Proc_state* proc_state = (Proc_state*)dstate;

    Proc_state_deinit(proc_state);

    if (proc_state->destroy != NULL)
        proc_state->destroy(dstate);
    else
        memory_free(proc_state);

    return;
}


