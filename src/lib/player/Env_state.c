

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Env_state.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <memory.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


struct Env_state
{
    const Environment* env;

    AAtree* vars;
};


Env_state* new_Env_state(const Environment* env)
{
    rassert(env != NULL);

    Env_state* estate = memory_alloc_item(Env_state);
    if (estate == NULL)
        return NULL;

    estate->env = env;
    estate->vars = NULL;

    return estate;
}


bool Env_state_refresh_space(Env_state* estate)
{
    rassert(estate != NULL);

    AAtree* vars = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)del_Env_var);
    if (vars == NULL)
        return false;

    Environment_iter* iter = Environment_iter_init(
            ENVIRONMENT_ITER_AUTO, estate->env);

    const char* name = Environment_iter_get_next_name(iter);
    while (name != NULL)
    {
        const Env_var* init_var = Environment_get(estate->env, name);

        Env_var* var = new_Env_var(Env_var_get_type(init_var), name);
        if (var == NULL || !AAtree_ins(vars, var))
        {
            del_Env_var(var);
            del_AAtree(vars);
            return false;
        }

        name = Environment_iter_get_next_name(iter);
    }

    del_AAtree(estate->vars);
    estate->vars = vars;

    Env_state_reset(estate);

    return true;
}


Env_var* Env_state_get_var(const Env_state* estate, const char* name)
{
    rassert(estate != NULL);
    rassert(name != NULL);

    if (estate->vars == NULL)
        return NULL;

    return AAtree_get_exact(estate->vars, name);
}


void Env_state_reset(Env_state* estate)
{
    rassert(estate != NULL);

    Environment_iter* iter = Environment_iter_init(ENVIRONMENT_ITER_AUTO, estate->env);

    const char* name = Environment_iter_get_next_name(iter);
    while (name != NULL)
    {
        const Env_var* init_var = Environment_get(estate->env, name);
        Env_var* state_var = Env_state_get_var(estate, name);
        rassert(init_var != NULL);
        rassert(state_var != NULL);

        Env_var_set_value(state_var, Env_var_get_value(init_var));

        name = Environment_iter_get_next_name(iter);
    }

    return;
}


void del_Env_state(Env_state* estate)
{
    if (estate == NULL)
        return;

    del_AAtree(estate->vars);
    memory_free(estate);

    return;
}


