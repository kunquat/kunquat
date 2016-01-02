

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_params.h>

#include <debug/assert.h>
#include <string/common.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define new_env_or_fail(env, nodes, xmin, xmax, xstep, ymin, ymax, ystep) \
    if (true)                                                             \
    {                                                                     \
        (env) = new_Envelope((nodes), (xmin), (xmax), (xstep),            \
                (ymin), (ymax), (ystep));                                 \
        if ((env) == NULL)                                                \
        {                                                                 \
            Au_params_deinit(aup);                                        \
            return NULL;                                                  \
        }                                                                 \
    } else (void)0

Au_params* Au_params_init(Au_params* aup, uint32_t device_id)
{
    assert(aup != NULL);
    assert(device_id > 0);

    aup->device_id = device_id;

    aup->force_volume_env = NULL;
    aup->force_pitch_env = NULL;
    aup->env_force = NULL;
    aup->env_force_rel = NULL;

    aup->volume = 1;
    aup->global_force = 1;
    aup->force = 0;
    aup->force_variation = 0;

    new_env_or_fail(aup->force_volume_env, 8,  0, 1, 0,  0, 1, 0);
    aup->force_volume_env_enabled = false;
    Envelope_set_node(aup->force_volume_env, 0, 0);
    Envelope_set_node(aup->force_volume_env, 1, 1);
    Envelope_set_first_lock(aup->force_volume_env, true, true);
    Envelope_set_last_lock(aup->force_volume_env, true, false);

    new_env_or_fail(aup->force_pitch_env, 8,  0, 1, 0,  -1, 1, 0);
    aup->force_pitch_env_enabled = false;
    Envelope_set_node(aup->force_pitch_env, 0, 0);
    Envelope_set_node(aup->force_pitch_env, 1, 0);
    Envelope_set_first_lock(aup->force_pitch_env, true, false);
    Envelope_set_last_lock(aup->force_pitch_env, true, false);

    new_env_or_fail(aup->env_force, 32,  0, INFINITY, 0,  0, 1, 0);
    aup->env_force_enabled = false;
    aup->env_force_loop_enabled = false;
    aup->env_force_carry = false;
    aup->env_force_scale_amount = 0;
    aup->env_force_center = 0;
    Envelope_set_node(aup->env_force, 0, 1);
    Envelope_set_node(aup->env_force, 1, 1);
    Envelope_set_first_lock(aup->env_force, true, false);

    new_env_or_fail(aup->env_force_rel, 32,  0, INFINITY, 0,  0, 1, 0);
    aup->env_force_rel_enabled = false;
    aup->env_force_rel_scale_amount = 0;
    aup->env_force_rel_center = 0;
    Envelope_set_node(aup->env_force_rel, 0, 1);
    Envelope_set_node(aup->env_force_rel, 1, 0);
    Envelope_set_first_lock(aup->env_force_rel, true, false);
    Envelope_set_last_lock(aup->env_force_rel, false, true);

    return aup;
}

#undef new_env_or_fail


typedef struct tdata
{
    Envelope* env;
    bool enabled;
    double scale_amount;
    double scale_center;
    bool carry;
    bool loop;
    const bool release;
} tdata;

static bool read_time_env(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    tdata* td = userdata;

    if (string_eq(key, "enabled"))
        Streader_read_bool(sr, &td->enabled);
    else if (string_eq(key, "scale_amount"))
        Streader_read_float(sr, &td->scale_amount);
    else if (string_eq(key, "scale_center"))
        Streader_read_float(sr, &td->scale_center);
    else if (string_eq(key, "envelope"))
        Envelope_read(td->env, sr);
    else if (!td->release && string_eq(key, "carry"))
        Streader_read_bool(sr, &td->carry);
    else if (!td->release && string_eq(key, "loop"))
        Streader_read_bool(sr, &td->loop);
    else
    {
        Streader_set_error(
                 sr, "Unrecognised key in the envelope: %s", key);
        return false;
    }

    return !Streader_is_error_set(sr);
}

static void parse_env_time(
        Streader* sr,
        tdata* td,
        double min_x, double max_x, double step_x,
        double min_y, double max_y, double step_y)
{
    assert(sr != NULL);
    assert(td != NULL);
    assert(td->env == NULL);

    if (Streader_is_error_set(sr))
        return;

    td->env = new_Envelope(32, min_x, max_x, step_x, min_y, max_y, step_y);
    if (td->env == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for envelope");
        return;
    }

    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_time_env, td))
        {
            del_Envelope(td->env);
            td->env = NULL;
            return;
        }
    }

    if (Envelope_node_count(td->env) == 0)
    {
        td->enabled = false;
        return;
    }

    int loop_start = Envelope_get_mark(td->env, 0);
    int loop_end = Envelope_get_mark(td->env, 1);
    if (td->release)
    {
        Envelope_set_mark(td->env, 0, -1);
        Envelope_set_mark(td->env, 1, -1);
    }
    else if (loop_start >= 0 || loop_end >= 0)
    {
        if (loop_start == -1)
            loop_start = 0;

        if (loop_end < loop_start)
            loop_end = loop_start;

        Envelope_set_mark(td->env, 0, loop_start);
        Envelope_set_mark(td->env, 1, loop_end);
    }

    return;
}


bool Au_params_parse_env_force(Au_params* aup, Streader* sr)
{
    assert(aup != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    tdata td =
    {
        .env = NULL,
        .enabled = false,
        .scale_amount = 0,
        .scale_center = 0,
        .carry = false,
        .loop = false,
        .release = false,
    };

    parse_env_time(sr, &td, 0, INFINITY, 0, 0, 1, 0);
    if (td.env == NULL)
        return false;

    assert(!Streader_is_error_set(sr));
    aup->env_force_enabled = td.enabled;
    aup->env_force_loop_enabled = td.loop;
    aup->env_force_scale_amount = td.scale_amount;
    aup->env_force_center = exp2(td.scale_center / 1200) * 440;
    aup->env_force_carry = td.carry;
    Envelope* old_env = aup->env_force;
    aup->env_force = td.env;
    del_Envelope(old_env);

    return true;
}


bool Au_params_parse_env_force_rel(Au_params* aup, Streader* sr)
{
    assert(aup != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    tdata td =
    {
        .env = NULL,
        .enabled = false,
        .scale_amount = 0,
        .scale_center = 0,
        .carry = false,
        .loop = false,
        .release = true,
    };

    parse_env_time(sr, &td, 0, INFINITY, 0, 0, 1, 0);
    if (td.env == NULL)
        return false;

    assert(!Streader_is_error_set(sr));
    aup->env_force_rel_enabled = td.enabled;
    aup->env_force_rel_scale_amount = td.scale_amount;
    aup->env_force_rel_center = exp2(td.scale_center / 1200) * 440;
    Envelope* old_env = aup->env_force_rel;
    aup->env_force_rel = td.env;
    del_Envelope(old_env);

    return true;
}


#define del_env_check(env)   \
    if (true)                \
    {                        \
        del_Envelope((env)); \
        (env) = NULL;        \
    } else (void)0

void Au_params_deinit(Au_params* aup)
{
    if (aup == NULL)
        return;

    del_env_check(aup->force_volume_env);
    del_env_check(aup->force_pitch_env);
    del_env_check(aup->env_force);
    del_env_check(aup->env_force_rel);

    return;
}

#undef del_env_check


