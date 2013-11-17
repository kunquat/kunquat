

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EXPR_H
#define K_EXPR_H


#include <player/Env_state.h>
#include <Streader.h>
#include <Value.h>
#include <Random.h>


/**
 * Evaluates an expression.
 *
 * \param sr       The expression reader -- must not be \c NULL.
 * \param estate   The Environment state -- must not be \c NULL.
 * \param state    The Read state -- must not be \c NULL.
 * \param meta     The meta variable, or \c NULL if not used.
 * \param res      A memory location for the result Value --
 *                 must not be \c NULL.
 * \param rand     A Random source -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if evaluation failed.
 */
bool evaluate_expr(
        Streader* sr,
        Env_state* estate,
        const Value* meta,
        Value* res,
        Random* rand);


#endif // K_EXPR_H


