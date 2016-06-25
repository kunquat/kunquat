

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EXPR_H
#define KQT_EXPR_H


#include <mathnum/Random.h>
#include <player/Env_state.h>
#include <string/Streader.h>
#include <Value.h>


/**
 * Evaluate an expression.
 *
 * \param sr       The expression reader -- must not be \c NULL.
 * \param estate   The Environment state, or \c NULL if environment is not used.
 * \param meta     The meta variable, or \c NULL if not used.
 * \param res      A memory location for the result Value --
 *                 must not be \c NULL.
 * \param rand     A Random source -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if evaluation failed.
 */
bool evaluate_expr(
        Streader* sr, Env_state* estate, const Value* meta, Value* res, Random* rand);


#endif // KQT_EXPR_H


