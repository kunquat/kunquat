

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>

#include <AAtree.h>
#include <Env_var.h>
#include <Environment.h>
#include <File_base.h>
#include <memory.h>
#include <xassert.h>


struct Environment
{
    AAtree* vars;
    AAiter* iter;
};


Environment_iter* Environment_iter_init(
        Environment_iter* iter,
        const Environment* env)
{
    assert(iter != NULL);
    assert(env != NULL);

    AAiter_change_tree(&iter->iter, env->vars);
    iter->next = AAiter_get_at_least(&iter->iter, "");

    return iter;
}


const char* Environment_iter_get_next_name(Environment_iter* iter)
{
    assert(iter != NULL);

    const char* next = iter->next;
    iter->next = AAiter_get_next(&iter->iter);

    return next;
}


Environment* new_Environment(void)
{
    Environment* env = memory_alloc_item(Environment);
    if (env == NULL)
        return NULL;

    env->vars = NULL;
    env->iter = NULL;
    env->vars = new_AAtree((int (*)(const void*, const void*))strcmp,
                           (void (*)(void*))del_Env_var);
    env->iter = new_AAiter(env->vars);
    if (env->vars == NULL || env->iter == NULL)
    {
        del_Environment(env);
        return NULL;
    }

    return env;
}


bool Environment_parse(Environment* env, char* str, Read_state* state)
{
    assert(env != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    if (str == NULL)
    {
        AAtree_clear(env->vars);
        AAiter_change_tree(env->iter, env->vars);
        return true;
    }

    str = read_const_char(str, '[', state);
    if (state->error)
        return false;

    str = read_const_char(str, ']', state);
    if (!state->error)
    {
        AAtree_clear(env->vars);
        AAiter_change_tree(env->iter, env->vars);
        return true;
    }

    Read_state_clear_error(state);
    AAtree* new_vars = new_AAtree(
            (int (*)(const void*, const void*))strcmp,
            (void (*)(void*))del_Env_var);
    if (new_vars == NULL)
        return false;

    bool expect_var = true;
    while (expect_var)
    {
        Env_var* var = new_Env_var_from_string(&str, state);
        if (var == NULL)
        {
            del_AAtree(new_vars);
            return false;
        }

        if (AAtree_contains(new_vars, Env_var_get_name(var)))
        {
            Read_state_set_error(state, "Variable name %s is not unique",
                                        Env_var_get_name(var));
            del_Env_var(var);
            del_AAtree(new_vars);
            return false;
        }

        if (!AAtree_ins(new_vars, var))
        {
            del_Env_var(var);
            del_AAtree(new_vars);
            return false;
        }

        check_next(str, state, expect_var);
    }

    AAiter_change_tree(env->iter, new_vars);
    AAtree* old_vars = env->vars;
    env->vars = new_vars;
    del_AAtree(old_vars);

    return true;
}


const Env_var* Environment_get(const Environment* env, const char* name)
{
    assert(env != NULL);
    assert(name != NULL);

    return AAtree_get_exact(env->vars, name);
}


void del_Environment(Environment* env)
{
    if (env == NULL)
        return;

    del_AAiter(env->iter);
    del_AAtree(env->vars);
    memory_free(env);

    return;
}


