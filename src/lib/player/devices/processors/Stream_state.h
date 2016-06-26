

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_STREAM_STATE_H
#define KQT_STREAM_STATE_H


#include <decl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>
#include <string/key_pattern.h>

#include <stdint.h>


Device_state* new_Stream_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);

bool Stream_pstate_set_init_value(
        Device_state* dstate, const Key_indices indices, double value);

bool Stream_pstate_set_init_osc_speed(
        Device_state* dstate, const Key_indices indices, double value);

bool Stream_pstate_set_init_osc_depth(
        Device_state* dstate, const Key_indices indices, double value);

void Stream_pstate_set_value(Device_state* dstate, double value);
void Stream_pstate_slide_target(Device_state* dstate, double value);
void Stream_pstate_slide_length(Device_state* dstate, const Tstamp* length);
void Stream_pstate_set_osc_speed(Device_state* dstate, double speed);
void Stream_pstate_set_osc_depth(Device_state* dstate, double depth);
void Stream_pstate_set_osc_speed_slide(Device_state* dstate, const Tstamp* length);
void Stream_pstate_set_osc_depth_slide(Device_state* dstate, const Tstamp* length);


Voice_state_get_size_func Stream_vstate_get_size;

void Stream_vstate_init(Voice_state* vstate, const Proc_state* proc_state);

const Linear_controls* Stream_vstate_get_controls(const Voice_state* vstate);

void Stream_vstate_set_controls(Voice_state* vstate, const Linear_controls* controls);


#endif // KQT_STREAM_STATE_H


