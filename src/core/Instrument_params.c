

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

#include <Instrument_params.h>


#define new_env_or_fail(env, nodes, xmin, xmax, xstep, ymin, ymax, ystep) \
    do\
    {\
        (env) = new_Envelope((nodes), (xmin), (xmax), (xstep),\
                (ymin), (ymax), (ystep));\
        if ((env) == NULL)\
        {\
            Instrument_params_uninit(ip);\
            return NULL;\
        }\
    } while (false)


Instrument_params* Instrument_params_init(Instrument_params* ip,
        frame_t** bufs,
        frame_t** vbufs,
        int buf_count,
        uint32_t buf_len)
{
    assert(ip != NULL);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(bufs[1] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs[1] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    ip->bufs = ip->gbufs = bufs;
    ip->buf_count = buf_count;
    ip->buf_len = buf_len;
    ip->pbufs = NULL;
    ip->vbufs = vbufs;
    ip->force_volume_env = NULL;
    ip->force_filter_env = NULL;
    ip->force_pitch_env = NULL;
    ip->volume_env = NULL;
    ip->volume_off_env = NULL;
    ip->pitch_pan_env = NULL;
    ip->filter_env = NULL;
    ip->filter_off_env = NULL;
    ip->notes = NULL;

    ip->pedal = 0;
    ip->volume = 1;

    new_env_or_fail(ip->force_volume_env, 8,  0, 1, 0,  0, 1, 0);
    ip->force_volume_env_enabled = false;
    Envelope_set_node(ip->force_volume_env, 0, 0);
    Envelope_set_node(ip->force_volume_env, 1, 1);
    Envelope_set_first_lock(ip->force_volume_env, true, true);
    Envelope_set_last_lock(ip->force_volume_env, true, false);

    new_env_or_fail(ip->force_filter_env, 8,  0, 1, 0,  0, 1, 0);
    ip->force_filter_env_enabled = false;
    Envelope_set_node(ip->force_filter_env, 0, 1);
    Envelope_set_node(ip->force_filter_env, 1, 1);
    Envelope_set_first_lock(ip->force_filter_env, true, false);
    Envelope_set_last_lock(ip->force_filter_env, true, false);

    new_env_or_fail(ip->force_pitch_env, 8,  0, 1, 0,  -1, 1, 0);
    ip->force_pitch_env_enabled = false;
    Envelope_set_node(ip->force_pitch_env, 0, 0);
    Envelope_set_node(ip->force_pitch_env, 1, 0);
    Envelope_set_first_lock(ip->force_pitch_env, true, false);
    Envelope_set_last_lock(ip->force_pitch_env, true, false);

    new_env_or_fail(ip->volume_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->volume_env_enabled = false;
    ip->volume_env_carry = false;
    ip->volume_env_scale = 1;
    ip->volume_env_center = 440;
    Envelope_set_node(ip->volume_env, 0, 1);
    Envelope_set_node(ip->volume_env, 1, 1);
    Envelope_set_first_lock(ip->volume_env, true, false);

    new_env_or_fail(ip->volume_off_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->volume_off_env_enabled = true;
    ip->volume_off_env_scale = 1;
    ip->volume_off_env_center = 440;
    Envelope_set_node(ip->volume_off_env, 0, 1);
    Envelope_set_node(ip->volume_off_env, 0.02, 0.3);
    Envelope_set_node(ip->volume_off_env, 1, 0);
    Envelope_set_first_lock(ip->volume_off_env, true, false);
    Envelope_set_last_lock(ip->volume_off_env, false, true);

    new_env_or_fail(ip->pitch_pan_env, 8,  -1, 1, 0,  -1, 1, 0);
    ip->pitch_pan_env_enabled = false;
    Envelope_set_node(ip->pitch_pan_env, -1, 0);
    Envelope_set_node(ip->pitch_pan_env, 0, 0);
    Envelope_set_node(ip->pitch_pan_env, 1, 0);
    Envelope_set_first_lock(ip->pitch_pan_env, true, false);
    Envelope_set_last_lock(ip->pitch_pan_env, true, false);

    new_env_or_fail(ip->filter_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->filter_env_enabled = false;
    ip->filter_env_scale = 1;
    ip->filter_env_center = 440;
    Envelope_set_node(ip->filter_env, 0, 1);
    Envelope_set_node(ip->filter_env, 1, 1);
    Envelope_set_first_lock(ip->filter_env, true, false);

    new_env_or_fail(ip->filter_off_env, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->filter_off_env_enabled = false;
    ip->filter_off_env_scale = 1;
    ip->filter_off_env_center = 440;
    Envelope_set_node(ip->filter_off_env, 0, 1);
    Envelope_set_node(ip->filter_off_env, 1, 1);
    Envelope_set_first_lock(ip->filter_off_env, true, false);

    return ip;
}


#define del_env_check(env)\
    do\
    {\
        if ((env) != NULL)\
        {\
            del_Envelope((env));\
            (env) = NULL;\
        }\
    } while (false)


void Instrument_params_uninit(Instrument_params* ip)
{
    assert(ip != NULL);
    del_env_check(ip->force_volume_env);
    del_env_check(ip->force_filter_env);
    del_env_check(ip->force_pitch_env);
    del_env_check(ip->volume_env);
    del_env_check(ip->volume_off_env);
    del_env_check(ip->pitch_pan_env);
    del_env_check(ip->filter_env);
    del_env_check(ip->filter_off_env);
    return;
}


