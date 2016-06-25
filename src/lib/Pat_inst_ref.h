

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


#ifndef KQT_PAT_INST_REF_H
#define KQT_PAT_INST_REF_H


#include <kunquat/limits.h>

#include <stdint.h>


// Pattern instance reference.
typedef struct Pat_inst_ref
{
    int16_t pat;
    int16_t inst;
} Pat_inst_ref;


/**
 * A new automatically allocated Pattern instance reference.
 */
#define PAT_INST_REF_AUTO (&(Pat_inst_ref){ .pat = 0, .inst = 0 })


/**
 * Helpers for printing Pattern instance references.
 */
#define PRIpi "(%" PRId16 ", %" PRId16 ")"
#define PRIVALpi(piref) (piref).pat, (piref).inst


/**
 * Compare two Pattern instance references.
 *
 * \param p1   The first Pattern instance reference -- must be valid.
 * \param p2   The second Pattern instance reference -- must be valid.
 *
 * \return   An integer less than, equal to or greater than zero if \a p1 is
 *           found, respectively, to be less than, equal to or greater than
 *           \a p2.
 */
int Pat_inst_ref_cmp(const Pat_inst_ref* p1, const Pat_inst_ref* p2);


#endif // KQT_PAT_INST_REF_H


