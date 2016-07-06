

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


#include <init/devices/param_types/Envelope.h>

#include <debug/assert.h>
#include <memory.h>
#include <string/common.h>

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


struct Envelope
{
    double min_x;
    double max_x;
    double step_x;
    double min_y;
    double max_y;
    double step_y;
    bool first_x_locked;
    bool first_y_locked;
    bool last_x_locked;
    bool last_y_locked;
    Envelope_int interp;
    int node_count;
    int nodes_max;
    int nodes_res;
    int marks[ENVELOPE_MARKS_MAX];
    double* nodes;
};


static bool Envelope_read_nodes(Envelope* env, Streader* sr);


Envelope* new_Envelope(int nodes_max,
        double min_x, double max_x, double step_x,
        double min_y, double max_y, double step_y)
{
    rassert(nodes_max > 1);
    rassert(!isnan(min_x));
    rassert(!isnan(max_x));
    rassert(step_x >= 0);
    rassert(!isnan(min_y));
    rassert(!isnan(max_y));
    rassert(step_y >= 0);

    Envelope* env = memory_alloc_item(Envelope);
    if (env == NULL)
        return NULL;

    env->nodes = memory_alloc_items(double, nodes_max * 2);
    if (env->nodes == NULL)
    {
        memory_free(env);
        return NULL;
    }

    env->node_count = 0;
    env->nodes_max = nodes_max;
    env->nodes_res = nodes_max;

    for (int i = 0; i < ENVELOPE_MARKS_MAX; ++i)
        env->marks[i] = -1;

    env->interp = ENVELOPE_INT_LINEAR;
    env->min_x = min_x;
    env->max_x = max_x;
    env->step_x = step_x;
    env->min_y = min_y;
    env->max_y = max_y;
    env->step_y = step_y;
    env->first_x_locked = false;
    env->first_y_locked = false;
    env->last_x_locked = false;
    env->last_y_locked = false;

    return env;
}


static bool read_env_mark(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);

    if (index >= ENVELOPE_MARKS_MAX)
    {
        Streader_set_error(sr, "Too many envelope marks");
        return false;
    }

    Envelope* env = userdata;

    int64_t value = -1;
    if (!Streader_read_int(sr, &value))
        return false;

    if (value < 0)
    {
        Streader_set_error(sr, "Envelope mark value must not be negative");
        return false;
    }
    if (value >= env->nodes_max)
    {
        Streader_set_error(
                sr,
                "Envelope mark value (%" PRId64
                    ") must not exceed maximum node count (%d)",
                value, env->nodes_max);
        return false;
    }

    Envelope_set_mark(env, index, (int)value);

    return true;
}

static bool read_env_item(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

    Envelope* env = userdata;

    if (string_eq(key, "nodes"))
    {
        return Envelope_read_nodes(env, sr);
    }
    else if (string_eq(key, "marks"))
    {
        if (!Streader_read_list(sr, read_env_mark, env))
            return false;
    }
    else if (string_eq(key, "smooth"))
    {
        bool smooth = false;
        if (!Streader_read_bool(sr, &smooth))
            return false;

        Envelope_set_interp(
                env, smooth ? ENVELOPE_INT_CURVE : ENVELOPE_INT_LINEAR);
    }
    else
    {
        Streader_set_error(
                sr, "Unrecognised key in the envelope: %s", key);
        return false;
    }

    return true;
}

bool Envelope_read(Envelope* env, Streader* sr)
{
    rassert(env != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Envelope_set_interp(env, ENVELOPE_INT_LINEAR);

    return Streader_read_dict(sr, read_env_item, env);
}


typedef struct ndata
{
    bool empty;
    Envelope* env;
} ndata;

static bool read_env_node(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    rassert(userdata != NULL);

    ndata* n = userdata;
    n->empty = false;
    Envelope* env = n->env;

    if (index >= env->nodes_max)
    {
        Streader_set_error(
                sr, "Too many envelope nodes (max %d)", env->nodes_max);
        return false;
    }

    double node[2] = { 0 };
    if (!Streader_readf(sr, "[%f,%f]", &node[0], &node[1]))
        return false;

    const int64_t read_pos = sr->pos;
    const bool is_last = Streader_try_match_char(sr, ']');
    sr->pos = read_pos;

    if (is_last && index < 1)
    {
        Streader_set_error(
                sr, "Not enough envelope nodes (at least 2 required)");
        return false;
    }

    if (index == 0)
    {
        Envelope_move_node(env, 0, node[0], node[1]);
    }
    else if (is_last)
    {
        Envelope_move_node(env, index, node[0], node[1]);
    }
    else
    {
        const int ret = Envelope_set_node(env, node[0], node[1]);
        if (ret == -1)
        {
            Streader_set_error(
                    sr,
                    "Node (%f,%f) outside valid range for this envelope"
                        " ((%f,%f)..(%f,%f))",
                    node[0], node[1],
                    env->min_x, env->min_y,
                    env->max_x, env->max_y);
            return false;
        }
    }

    return true;
}

static bool Envelope_read_nodes(Envelope* env, Streader* sr)
{
    rassert(env != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    double min_x = !isfinite(env->min_x) ? -DBL_MAX : env->min_x;
    double min_y = !isfinite(env->min_y) ? -DBL_MAX : env->min_y;
    double max_x = !isfinite(env->max_x) ? DBL_MAX : env->max_x;
    double max_y = !isfinite(env->max_x) ? DBL_MAX : env->max_y;
    Envelope_set_node(env, min_x, min_y);
    Envelope_set_node(env, max_x, max_y);

    ndata n = { true, env };

    if (!Streader_read_list(sr, read_env_node, &n))
        return false;

    if (n.empty)
    {
        Streader_set_error(sr, "Node list is empty");
        return false;
    }

    return true;
}


int Envelope_node_count(const Envelope* env)
{
    rassert(env != NULL);
    return env->node_count;
}


void Envelope_set_interp(Envelope* env, Envelope_int interp)
{
    rassert(env != NULL);
//  rassert(interp >= ENVELOPE_INT_NEAREST);
    rassert(interp < ENVELOPE_INT_LAST);
    env->interp = interp;
    return;
}


void Envelope_set_mark(Envelope* env, int index, int value)
{
    rassert(env != NULL);
    rassert(index >= 0);
    rassert(index < ENVELOPE_MARKS_MAX);
    env->marks[index] = value;
    return;
}


int Envelope_get_mark(const Envelope* env, int index)
{
    rassert(env != NULL);
    rassert(index >= 0);
    rassert(index < ENVELOPE_MARKS_MAX);

    return env->marks[index];
}


Envelope_int Envelope_get_interp(const Envelope* env)
{
    rassert(env != NULL);
    return env->interp;
}


int Envelope_set_node(Envelope* env, double x, double y)
{
    rassert(env != NULL);
    rassert(isfinite(x));
    rassert(isfinite(y));

    if (env->node_count >= env->nodes_max)
    {
        rassert(env->node_count == env->nodes_max);
        return -1;
    }

    if (env->node_count > 0)
    {
        if (x < env->nodes[0] && (env->first_x_locked || env->first_y_locked))
            return -1;

        if (x > env->nodes[env->node_count * 2 - 2]
                && (env->last_x_locked || env->last_y_locked))
            return -1;
    }

    int start = 0;
    int end = env->node_count - 1;

    while (start <= end)
    {
        int middle = (start + end) / 2;
        if (env->nodes[middle * 2] < x)
            start = middle + 1;
        else if (env->nodes[middle * 2] > x)
            end = middle - 1;
        else
        {
            Envelope_move_node(env, middle, x, y);
            return middle;
        }
    }

    ++env->node_count;

    for (int i = start + 1; i < env->node_count; ++i)
    {
        env->nodes[i * 2] = env->nodes[i * 2 - 2];
        env->nodes[i * 2 + 1] = env->nodes[i * 2 - 1];
    }

    env->nodes[start * 2] = x;
    env->nodes[start * 2 + 1] = y;

    for (int i = 0; i < ENVELOPE_MARKS_MAX; ++i)
    {
        if (env->marks[i] >= start)
            ++env->marks[i];
    }

    return start;
}


bool Envelope_del_node(Envelope* env, int index)
{
    rassert(env != NULL);
    rassert(index >= 0);

    if (index >= env->node_count)
        return false;

    if (index == 0 && (env->first_x_locked || env->first_y_locked))
        return false;

    if (index == env->node_count - 1
            && (env->last_x_locked || env->last_y_locked))
        return false;

    for (int i = index * 2; i < env->node_count * 2 - 2; i += 2)
    {
        env->nodes[i] = env->nodes[i + 2];
        env->nodes[i + 1] = env->nodes[i + 3];
    }

    --env->node_count;

    for (int i = 0; i < ENVELOPE_MARKS_MAX; ++i)
    {
        if (env->marks[i] > index)
            --env->marks[i];
    }

    return true;
}


double* Envelope_get_node(const Envelope* env, int index)
{
    rassert(env != NULL);
    rassert(index >= 0);

    if (index >= env->node_count)
        return NULL;

    return env->nodes + index * 2;
}


double* Envelope_move_node(Envelope* env, int index, double x, double y)
{
    rassert(env != NULL);
    rassert(index >= 0);
    rassert(isfinite(x));
    rassert(isfinite(y));

    if (index >= env->node_count)
        return NULL;

    double max_x = env->max_x;
    double max_y = env->max_y;
    double min_x = env->min_x;
    double min_y = env->min_y;

    if (index == env->node_count - 1)
    {
        if (env->last_x_locked)
            min_x = max_x = env->nodes[env->node_count * 2 - 2];

        if (env->last_y_locked)
            min_y = max_y = env->nodes[env->node_count * 2 - 1];
    }
    else
    {
        double next_x = env->nodes[index * 2 + 2];
        max_x = next_x - env->step_x;
        if (max_x >= next_x)
            max_x = nextafter(max_x, -INFINITY);
    }

    if (index == 0)
    {
        if (env->first_x_locked)
            min_x = max_x = env->nodes[0];

        if (env->first_y_locked)
            min_y = max_y = env->nodes[1];
    }
    else
    {
        double prev_x = env->nodes[index * 2 - 2];
        min_x = prev_x + env->step_x;
        if (min_x <= prev_x)
            min_x = nextafter(min_x, INFINITY);
    }

    if (min_x > max_x)
        return env->nodes + index * 2;

    if (min_x > x)
        env->nodes[index * 2] = min_x;
    else if (max_x < x)
        env->nodes[index * 2] = max_x;
    else
        env->nodes[index * 2] = x;

    if (min_y > y)
        env->nodes[index * 2 + 1] = min_y;
    else if (max_y < y)
        env->nodes[index * 2 + 1] = max_y;
    else
        env->nodes[index * 2 + 1] = y;

    return env->nodes + index * 2;
}


double Envelope_get_value(const Envelope* env, double x)
{
    rassert(env != NULL);
    rassert(isfinite(x));

    if (env->node_count == 0
            || x < env->nodes[0]
            || x > env->nodes[env->node_count * 2 - 2])
        return NAN;

    // Binary search the markers surrounding x
    int start = 0;
    int end = env->node_count - 1;
    while (start <= end)
    {
        int middle = (start + end) / 2;
        if (env->nodes[middle * 2] < x)
            start = middle + 1;
        else if (env->nodes[middle * 2] > x)
            end = middle - 1;
        else
            return env->nodes[middle * 2 + 1];
    }

    // Interpolate
    rassert(start < env->node_count);
    rassert(end >= 0);
    rassert(start == end + 1);

    double prev_x = env->nodes[end * 2];
    double prev_y = env->nodes[end * 2 + 1];
    double next_x = env->nodes[start * 2];
    double next_y = env->nodes[start * 2 + 1];

    switch (env->interp)
    {
        case ENVELOPE_INT_NEAREST:
        {
            if (x - prev_x < next_x - x)
                return prev_y;
            else
                return next_y;
        }
        break;

        case ENVELOPE_INT_LINEAR:
        {
            return prev_y + (x - prev_x) *
                   ((next_y - prev_y) / (next_x - prev_x));
        }
        break;

        default:
            rassert(false);
    }

    rassert(false);
    return NAN;
}


void Envelope_set_first_lock(Envelope* env, bool lock_x, bool lock_y)
{
    rassert(env != NULL);
    rassert(env->node_count > 0);

    env->first_x_locked = lock_x;
    env->first_y_locked = lock_y;

    return;
}


void Envelope_set_last_lock(Envelope* env, bool lock_x, bool lock_y)
{
    rassert(env != NULL);
    rassert(env->node_count > 0);

    env->last_x_locked = lock_x;
    env->last_y_locked = lock_y;

    return;
}


void del_Envelope(Envelope* env)
{
    if (env == NULL)
        return;

    memory_free(env->nodes);
    memory_free(env);

    return;
}


