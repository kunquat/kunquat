

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


#ifndef KQT_JUMP_CONTEXT_H
#define KQT_JUMP_CONTEXT_H


#include <mathnum/Tstamp.h>
#include <Pat_inst_ref.h>

#include <stdint.h>
#include <stdlib.h>


/**
 * Playback state information of a jump.
 */
typedef struct Jump_context
{
    // Location where the jump takes place
    Pat_inst_ref piref;
    Tstamp row;
    int ch_num;
    int64_t order;

    int64_t counter;

    // Jump target information
    Pat_inst_ref target_piref;
    Tstamp target_row;
} Jump_context;


#define JUMP_CONTEXT_AUTO (&(Jump_context){ \
        .piref = *PAT_INST_REF_AUTO, .row = { 0, 0 } })


/**
 * Create a new Jump context.
 *
 * \return   The new Jump context if successful, or \c NULL if memory
 *           allocation failed.
 */
Jump_context* new_Jump_context(void);


/**
 * Compare two Jump contexts.
 *
 * \param jc1   The first Jump context -- must not be \c NULL.
 * \param jc2   The second Jump context -- must not be \c NULL.
 *
 * \return   An integer less, equal to, or greater than zero if \a jc1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a jc2.
 */
int Jump_context_cmp(const Jump_context* jc1, const Jump_context* jc2);


/**
 * Destroy an existing Jump context.
 *
 * \param jc   The Jump context, or \c NULL.
 */
void del_Jump_context(Jump_context* jc);


#endif // KQT_JUMP_CONTEXT_H


