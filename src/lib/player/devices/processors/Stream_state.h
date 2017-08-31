

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
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


#include <init/devices/Device_impl.h>
#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>


Device_state_create_func new_Stream_pstate;

Set_state_float_func Stream_pstate_set_init_value;
Set_state_float_func Stream_pstate_set_init_osc_speed;
Set_state_float_func Stream_pstate_set_init_osc_depth;

void Stream_pstate_set_value(Device_state* dstate, double value);
void Stream_pstate_slide_target(Device_state* dstate, double value);
void Stream_pstate_slide_length(Device_state* dstate, const Tstamp* length);
void Stream_pstate_set_osc_speed(Device_state* dstate, double speed);
void Stream_pstate_set_osc_depth(Device_state* dstate, double depth);
void Stream_pstate_set_osc_speed_slide(Device_state* dstate, const Tstamp* length);
void Stream_pstate_set_osc_depth_slide(Device_state* dstate, const Tstamp* length);


Voice_state_get_size_func Stream_vstate_get_size;
Voice_state_init_func Stream_vstate_init;
Voice_state_render_voice_func Stream_vstate_render_voice;

const Linear_controls* Stream_vstate_get_controls(const Voice_state* vstate);

void Stream_vstate_set_controls(Voice_state* vstate, const Linear_controls* controls);


#endif // KQT_STREAM_STATE_H


