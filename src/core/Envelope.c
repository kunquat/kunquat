

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
#include <stdio.h>
#include <math.h>

#include <Envelope.h>

#include <xmemory.h>


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


Envelope* new_Envelope(int nodes_max,
        double min_x, double max_x, double step_x,
        double min_y, double max_y, double step_y)
{
    assert(nodes_max > 1);
    assert(!isnan(min_x));
    assert(!isnan(max_x));
    assert(step_x >= 0);
    assert(!isnan(min_y));
    assert(!isnan(max_y));
    assert(step_y >= 0);
    Envelope* env = xalloc(Envelope);
    if (env == NULL)
    {
        return NULL;
    }
    env->nodes = xnalloc(double, nodes_max * 2);
    if (env->nodes == NULL)
    {
        xfree(env);
        return NULL;
    }
    env->node_count = 0;
    env->nodes_max = nodes_max;
    env->nodes_res = nodes_max;
    for (int i = 0; i < ENVELOPE_MARKS_MAX; ++i)
    {
        env->marks[i] = -1;
    }
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


int Envelope_node_count(Envelope* env)
{
    assert(env != NULL);
    return env->node_count;
}


void Envelope_set_interp(Envelope* env, Envelope_int interp)
{
    assert(env != NULL);
//  assert(interp >= ENVELOPE_INT_NEAREST);
    assert(interp < ENVELOPE_INT_LAST);
    env->interp = interp;
    return;
}


void Envelope_set_mark(Envelope* env, int index, int value)
{
    assert(env != NULL);
    assert(index >= 0);
    assert(index < ENVELOPE_MARKS_MAX);
    env->marks[index] = value;
    return;
}


int Envelope_get_mark(Envelope* env, int index)
{
    assert(env != NULL);
    assert(index >= 0);
    assert(index < ENVELOPE_MARKS_MAX);
    return env->marks[index];
}


Envelope_int Envelope_get_interp(Envelope* env)
{
    assert(env != NULL);
    return env->interp;
}


int Envelope_set_node(Envelope* env, double x, double y)
{
    assert(env != NULL);
    assert(isfinite(x));
    assert(isfinite(y));
    if (env->node_count >= env->nodes_max)
    {
        assert(env->node_count == env->nodes_max);
        return -1;
    }
    if (env->node_count > 0)
    {
        if (x < env->nodes[0] && (env->first_x_locked || env->first_y_locked))
        {
            return -1;
        }
        if (x > env->nodes[env->node_count * 2 - 2]
                && (env->last_x_locked || env->last_y_locked))
        {
            return -1;
        }
    }
    int start = 0;
    int end = env->node_count - 1;
    while (start <= end)
    {
        int middle = (start + end) / 2;
        if (env->nodes[middle * 2] < x)
        {
            start = middle + 1;
        }
        else if (env->nodes[middle * 2] > x)
        {
            end = middle - 1;
        }
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
        {
            ++env->marks[i];
        }
    }
    return start;
}


bool Envelope_del_node(Envelope* env, int index)
{
    assert(env != NULL);
    assert(index >= 0);
    if (index >= env->node_count)
    {
        return false;
    }
    if (index == 0 && (env->first_x_locked || env->first_y_locked))
    {
        return false;
    }
    if (index == env->node_count - 1
            && (env->last_x_locked || env->last_y_locked))
    {
        return false;
    }
    for (int i = index * 2; i < env->node_count * 2 - 2; i += 2)
    {
        env->nodes[i] = env->nodes[i + 2];
        env->nodes[i + 1] = env->nodes[i + 3];
    }
    --env->node_count;
    for (int i = 0; i < ENVELOPE_MARKS_MAX; ++i)
    {
        if (env->marks[i] > index)
        {
            --env->marks[i];
        }
    }
    return true;
}


double* Envelope_get_node(Envelope* env, int index)
{
    assert(env != NULL);
    assert(index >= 0);
    if (index >= env->node_count)
    {
        return NULL;
    }
    return env->nodes + index * 2;
}


double* Envelope_move_node(Envelope* env, int index, double x, double y)
{
    assert(env != NULL);
    assert(index >= 0);
    assert(isfinite(x));
    assert(isfinite(y));
    if (index >= env->node_count)
    {
        return NULL;
    }
    double max_x = env->max_x;
    double max_y = env->max_y;
    double min_x = env->min_x;
    double min_y = env->min_y;
    if (index == env->node_count - 1)
    {
        if (env->last_x_locked)
        {
            min_x = max_x = env->nodes[env->node_count * 2 - 2];
        }
        if (env->last_y_locked)
        {
            min_y = max_y = env->nodes[env->node_count * 2 - 1];
        }
    }
    else
    {
        double next_x = env->nodes[index * 2 + 2];
        max_x = next_x - env->step_x;
        if (max_x >= next_x)
        {
            max_x = nextafter(max_x, -INFINITY);
        }
    }
    if (index == 0)
    {
        if (env->first_x_locked)
        {
            min_x = max_x = env->nodes[0];
        }
        if (env->first_y_locked)
        {
            min_y = max_y = env->nodes[1];
        }
    }
    else
    {
        double prev_x = env->nodes[index * 2 - 2];
        min_x = prev_x + env->step_x;
        if (min_x <= prev_x)
        {
            min_x = nextafter(min_x, INFINITY);
        }
    }
    if (min_x > max_x)
    {
        return env->nodes + index * 2;
    }
    if (min_x > x)
    {
        env->nodes[index * 2] = min_x;
    }
    else if (max_x < x)
    {
        env->nodes[index * 2] = max_x;
    }
    else
    {
        env->nodes[index * 2] = x;
    }
    if (min_y > y)
    {
        env->nodes[index * 2 + 1] = min_y;
    }
    else if (max_y < y)
    {
        env->nodes[index * 2 + 1] = max_y;
    }
    else
    {
        env->nodes[index * 2 + 1] = y;
    }
    return env->nodes + index * 2;
}


double Envelope_get_value(Envelope* env, double x)
{
    assert(env != NULL);
    assert(isfinite(x));
    if (env->node_count == 0
            || x < env->nodes[0]
            || x > env->nodes[env->node_count * 2 - 2])
    {
        return NAN;
    }
    int start = 0;
    int end = env->node_count - 1;
    while (start <= end)
    {
        int middle = (start + end) / 2;
        if (env->nodes[middle * 2] < x)
        {
            start = middle + 1;
        }
        else if (env->nodes[middle * 2] > x)
        {
            end = middle - 1;
        }
        else
        {
            return env->nodes[middle * 2];
        }
    }
    assert(start < env->node_count);
    assert(end >= 0);
    assert(start == end + 1);
    double prev_x = env->nodes[end * 2];
    double prev_y = env->nodes[end * 2 + 1];
    double next_x = env->nodes[start * 2];
    double next_y = env->nodes[start * 2 + 1];
    switch (env->interp)
    {
        case ENVELOPE_INT_NEAREST:
        {
            if (x - prev_x < next_x - x)
            {
                return prev_y;
            }
            else
            {
                return next_y;
            }
        } break;
        case ENVELOPE_INT_LINEAR:
        {
            double interval = next_x - prev_x;
            double y = prev_y * (next_x - x) / interval
                    + next_y * (x - prev_x) / interval;
            return y;
        } break;
        default:
            assert(false);
    }
    assert(false);
    return NAN;
}


void Envelope_set_first_lock(Envelope* env, bool lock_x, bool lock_y)
{
    assert(env != NULL);
    assert(env->node_count > 0);
    env->first_x_locked = lock_x;
    env->first_y_locked = lock_y;
    return;
}


void Envelope_set_last_lock(Envelope* env, bool lock_x, bool lock_y)
{
    assert(env != NULL);
    assert(env->node_count > 0);
    env->last_x_locked = lock_x;
    env->last_y_locked = lock_y;
    return;
}


void del_Envelope(Envelope* env)
{
    assert(env != NULL);
    assert(env->nodes != NULL);
    xfree(env->nodes);
    xfree(env);
    return;
}


