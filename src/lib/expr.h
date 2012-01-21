

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
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


#include <Environment.h>
#include <File_base.h>
#include <Value.h>


/**
 * Evaluates an expression.
 *
 * \param str     The expression -- must not be \c NULL.
 * \param env     The Environment -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 * \param meta    The meta variable, or \c NULL if not used.
 * \param res     A memory location for the result Value --
 *                must not be \c NULL.
 *
 * \return   The position of \a str after parsing.
 */
char* evaluate_expr(char* str,
                    Environment* env,
                    Read_state* state,
                    Value* meta,
                    Value* res);


#endif // K_EXPR_H


