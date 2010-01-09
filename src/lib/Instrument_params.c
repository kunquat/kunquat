

/*
 * Author: Tomi Jylhä-Ollila, Finland, 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat waivers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from Finland.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <Instrument_params.h>
#include <File_base.h>


#define new_env_or_fail(env, nodes, xmin, xmax, xstep, ymin, ymax, ystep) \
    if (true)                                                             \
    {                                                                     \
        (env) = new_Envelope((nodes), (xmin), (xmax), (xstep),            \
                (ymin), (ymax), (ystep));                                 \
        if ((env) == NULL)                                                \
        {                                                                 \
            Instrument_params_uninit(ip);                                 \
            return NULL;                                                  \
        }                                                                 \
    } else (void)0

Instrument_params* Instrument_params_init(Instrument_params* ip,
                                          kqt_frame** bufs,
                                          kqt_frame** vbufs,
                                          kqt_frame** vbufs2,
                                          int buf_count,
                                          uint32_t buf_len,
                                          Scale*** scale)
{
    assert(ip != NULL);
    assert(bufs != NULL);
    assert(bufs[0] != NULL);
    assert(vbufs != NULL);
    assert(vbufs[0] != NULL);
    assert(vbufs2 != NULL);
    assert(vbufs2[0] != NULL);
    assert(buf_count > 0);
    assert(buf_len > 0);
    assert(scale != NULL);
    assert(*scale != NULL);
    ip->bufs = ip->gbufs = bufs;
    ip->buf_count = buf_count;
    ip->buf_len = buf_len;
    ip->pbufs = NULL;
    ip->vbufs = vbufs;
    ip->vbufs2 = vbufs2;
    ip->force_volume_env = NULL;
    ip->force_filter_env = NULL;
    ip->force_pitch_env = NULL;
    ip->volume_env = NULL;
    ip->volume_off_env = NULL;
    ip->pitch_pan_env = NULL;
    ip->filter_env = NULL;
    ip->filter_off_env = NULL;
    ip->scale = scale;

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
    ip->volume_off_env_enabled = false;
    ip->volume_off_env_factor = 1;
    ip->volume_off_env_center = 440;
    Envelope_set_node(ip->volume_off_env, 0, 1);
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

#undef new_env_or_fail


bool Instrument_params_parse_env_vol_rel(Instrument_params* ip,
                                         char* str,
                                         Read_state* state)
{
    assert(ip != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    bool enabled = false;
    double scale_factor = 1;
    double scale_center = 0;
    Envelope* env = new_Envelope(32, 0, INFINITY, 0, 0, 1, 0);
    if (env == NULL)
    {
        return false;
    }
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            del_Envelope(env);
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
                    del_Envelope(env);
                    return false;
                }
                if (strcmp(key, "enabled") == 0)
                {
                    str = read_bool(str, &enabled, state);
                }
                else if (strcmp(key, "scale_factor") == 0)
                {
                    str = read_double(str, &scale_factor, state);
                }
                else if (strcmp(key, "scale_center") == 0)
                {
                    str = read_double(str, &scale_center, state);
                }
                else if (strcmp(key, "nodes") == 0)
                {
                    str = Envelope_read(env, str, state);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unrecognised key in Note Off volume envelope: %s", key);
                    del_Envelope(env);
                    return false;
                }
                if (state->error)
                {
                    del_Envelope(env);
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                del_Envelope(env);
                return false;
            }
        }
    }
    ip->volume_off_env_enabled = enabled;
    ip->volume_off_env_factor = scale_factor;
    ip->volume_off_env_center = scale_center;
    Envelope* old_env = ip->volume_off_env;
    ip->volume_off_env = env;
    del_Envelope(old_env);
    return true;
}


#define del_env_check(env)       \
    if (true)                    \
    {                            \
        if ((env) != NULL)       \
        {                        \
            del_Envelope((env)); \
            (env) = NULL;        \
        }                        \
    } else (void)0

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

#undef del_env_check


