

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <debug/assert.h>
#include <devices/Instrument_params.h>
#include <string/common.h>


#define new_env_or_fail(env, nodes, xmin, xmax, xstep, ymin, ymax, ystep) \
    if (true)                                                             \
    {                                                                     \
        (env) = new_Envelope((nodes), (xmin), (xmax), (xstep),            \
                (ymin), (ymax), (ystep));                                 \
        if ((env) == NULL)                                                \
        {                                                                 \
            Instrument_params_deinit(ip);                                 \
            return NULL;                                                  \
        }                                                                 \
    } else (void)0

Instrument_params* Instrument_params_init(
        Instrument_params* ip,
        uint32_t device_id)
{
    assert(ip != NULL);
    assert(device_id > 0);

    ip->device_id = device_id;

    ip->force_volume_env = NULL;
    ip->env_force_filter = NULL;
    ip->force_pitch_env = NULL;
    ip->env_force = NULL;
    ip->env_force_rel = NULL;
    ip->env_pitch_pan = NULL;
    ip->filter_env = NULL;
    ip->filter_off_env = NULL;

    ip->volume = 1;
    ip->global_force = 1;
    ip->force = 0;
    ip->force_variation = 0;

    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ip->pitch_locks[i].enabled = false;
        ip->pitch_locks[i].cents = 0;
        ip->pitch_locks[i].freq = exp2(0 / 1200.0) * 440;
    }
#if 0
    ip->pitch_lock_enabled = false;
    ip->pitch_lock_cents = 0;
    ip->pitch_lock_freq = exp2(ip->pitch_lock_cents / 1200.0) * 440;
#endif

    new_env_or_fail(ip->force_volume_env, 8,  0, 1, 0,  0, 1, 0);
    ip->force_volume_env_enabled = false;
    Envelope_set_node(ip->force_volume_env, 0, 0);
    Envelope_set_node(ip->force_volume_env, 1, 1);
    Envelope_set_first_lock(ip->force_volume_env, true, true);
    Envelope_set_last_lock(ip->force_volume_env, true, false);

    new_env_or_fail(ip->env_force_filter, 8,  0, 1, 0,  0, 1, 0);
    ip->env_force_filter_enabled = false;
    Envelope_set_node(ip->env_force_filter, 0, 1);
    Envelope_set_node(ip->env_force_filter, 1, 1);
    Envelope_set_first_lock(ip->env_force_filter, true, false);
    Envelope_set_last_lock(ip->env_force_filter, true, false);

    new_env_or_fail(ip->force_pitch_env, 8,  0, 1, 0,  -1, 1, 0);
    ip->force_pitch_env_enabled = false;
    Envelope_set_node(ip->force_pitch_env, 0, 0);
    Envelope_set_node(ip->force_pitch_env, 1, 0);
    Envelope_set_first_lock(ip->force_pitch_env, true, false);
    Envelope_set_last_lock(ip->force_pitch_env, true, false);

    new_env_or_fail(ip->env_force, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->env_force_enabled = false;
    ip->env_force_loop_enabled = false;
    ip->env_force_carry = false;
    ip->env_force_scale_amount = 0;
    ip->env_force_center = 0;
    Envelope_set_node(ip->env_force, 0, 1);
    Envelope_set_node(ip->env_force, 1, 1);
    Envelope_set_first_lock(ip->env_force, true, false);

    new_env_or_fail(ip->env_force_rel, 32,  0, INFINITY, 0,  0, 1, 0);
    ip->env_force_rel_enabled = false;
    ip->env_force_rel_scale_amount = 0;
    ip->env_force_rel_center = 0;
    Envelope_set_node(ip->env_force_rel, 0, 1);
    Envelope_set_node(ip->env_force_rel, 1, 0);
    Envelope_set_first_lock(ip->env_force_rel, true, false);
    Envelope_set_last_lock(ip->env_force_rel, false, true);

    new_env_or_fail(ip->env_pitch_pan, 8,  -6000, 6000, 0,  -1, 1, 0);
    ip->env_pitch_pan_enabled = false;
    Envelope_set_node(ip->env_pitch_pan, -1, 0);
    Envelope_set_node(ip->env_pitch_pan, 0, 0);
    Envelope_set_node(ip->env_pitch_pan, 1, 0);
    Envelope_set_first_lock(ip->env_pitch_pan, true, false);
    Envelope_set_last_lock(ip->env_pitch_pan, true, false);

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


typedef struct ntdata
{
    bool enabled;
    bool nodes_found;
    const char* type;
    Envelope* env;
} ntdata;

static bool read_nontime_env(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    ntdata* d = userdata;

    if (string_eq(key, "enabled"))
    {
        if (!Streader_read_bool(sr, &d->enabled))
            return false;
    }
    else if (string_eq(key, "envelope"))
    {
        if (!Envelope_read(d->env, sr))
            return false;
        d->nodes_found = true;
    }
    else
    {
        Streader_set_error(
                 sr, "Unrecognised key in %s envelope: %s", d->type, key);
        return false;
    }

    return true;
}

bool Instrument_params_parse_env_force_filter(
        Instrument_params* ip, Streader* sr)
{
    assert(ip != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Envelope* env = new_Envelope(8, 0, 1, 0, 0, 1, 0);
    if (env == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for force-filter envelope");
        return false;
    }

    ntdata d =
    {
        .enabled = false,
        .nodes_found = false,
        .type = "force-filter",
        .env = env,
    };

    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_nontime_env, &d))
        {
            del_Envelope(env);
            return false;
        }
    }

    ip->env_force_filter_enabled = d.enabled;
    Envelope* old_env = ip->env_force_filter;
    ip->env_force_filter = env;
    del_Envelope(old_env);

    if (!d.nodes_found)
    {
        assert(Envelope_node_count(env) == 0);
        int index = Envelope_set_node(env, 0, 1);
        assert(index == 0);
        index = Envelope_set_node(env, 1, 1);
        assert(index == 1);
        (void)index;
    }

    return true;
}


bool Instrument_params_parse_env_pitch_pan(
        Instrument_params* ip, Streader* sr)
{
    assert(ip != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Envelope* env = new_Envelope(32, -6000, 6000, 0, -1, 1, 0);
    if (env == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for pitch-pan envelope");
        return false;
    }

    ntdata d =
    {
        .enabled = false,
        .nodes_found = false,
        .type = "pitch-pan",
        .env = env,
    };

    if (Streader_has_data(sr))
    {
        if (!Streader_read_dict(sr, read_nontime_env, &d))
        {
            del_Envelope(env);
            return false;
        }
    }

    ip->env_pitch_pan_enabled = d.enabled;
    Envelope* old_env = ip->env_pitch_pan;
    ip->env_pitch_pan = env;
    del_Envelope(old_env);

    if (!d.nodes_found)
    {
        assert(Envelope_node_count(env) == 0);
        int index = Envelope_set_node(env, -6000, 0);
        assert(index == 0);
        index = Envelope_set_node(env, 6000, 0);
        assert(index == 1);
        (void)index;
    }

    return true;
}


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

static void parse_env_time(Streader* sr, tdata* td)
{
    assert(sr != NULL);
    assert(td != NULL);
    assert(td->env == NULL);

    if (Streader_is_error_set(sr))
        return;

    td->env = new_Envelope(32, 0, INFINITY, 0, 0, 1, 0);
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


bool Instrument_params_parse_env_force(
        Instrument_params* ip, Streader* sr)
{
    assert(ip != NULL);
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

    parse_env_time(sr, &td);
    if (td.env == NULL)
        return false;

    assert(!Streader_is_error_set(sr));
    ip->env_force_enabled = td.enabled;
    ip->env_force_loop_enabled = td.loop;
    ip->env_force_scale_amount = td.scale_amount;
    ip->env_force_center = exp2(td.scale_center / 1200) * 440;
    ip->env_force_carry = td.carry;
    Envelope* old_env = ip->env_force;
    ip->env_force = td.env;
    del_Envelope(old_env);

    return true;
}


bool Instrument_params_parse_env_force_rel(
        Instrument_params* ip, Streader* sr)
{
    assert(ip != NULL);
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

    parse_env_time(sr, &td);
    if (td.env == NULL)
        return false;

    assert(!Streader_is_error_set(sr));
    ip->env_force_rel_enabled = td.enabled;
    ip->env_force_rel_scale_amount = td.scale_amount;
    ip->env_force_rel_center = exp2(td.scale_center / 1200) * 440;
    Envelope* old_env = ip->env_force_rel;
    ip->env_force_rel = td.env;
    del_Envelope(old_env);

    return true;
}


#define del_env_check(env)   \
    if (true)                \
    {                        \
        del_Envelope((env)); \
        (env) = NULL;        \
    } else (void)0

void Instrument_params_deinit(Instrument_params* ip)
{
    if (ip == NULL)
        return;

    del_env_check(ip->force_volume_env);
    del_env_check(ip->env_force_filter);
    del_env_check(ip->force_pitch_env);
    del_env_check(ip->env_force);
    del_env_check(ip->env_force_rel);
    del_env_check(ip->env_pitch_pan);
    del_env_check(ip->filter_env);
    del_env_check(ip->filter_off_env);

    return;
}

#undef del_env_check


