

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


#include <init/sheet/Pattern.h>

#include <containers/Bit_array.h>
#include <debug/assert.h>
#include <memory.h>
#include <string/common.h>

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


struct Pattern
{
    Column* cols[KQT_COLUMNS_MAX];
    Tstamp length;
    Bit_array* existents;
};


Pattern* new_Pattern(void)
{
    Pattern* pat = memory_alloc_item(Pattern);
    if (pat == NULL)
        return NULL;

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
        pat->cols[i] = NULL;
    pat->existents = NULL;

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        pat->cols[i] = new_Column(NULL);
        if (pat->cols[i] == NULL)
        {
            del_Pattern(pat);
            return NULL;
        }
    }

    pat->existents = new_Bit_array(KQT_PAT_INSTANCES_MAX);
    if (pat->existents == NULL)
    {
        del_Pattern(pat);
        return NULL;
    }

    Tstamp_set(&pat->length, 16, 0);

    return pat;
}


bool Pattern_read_length(Pattern* pat, Streader* sr)
{
    rassert(pat != NULL);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    Tstamp* length = PATTERN_DEFAULT_LENGTH;

    if (Streader_has_data(sr) && !Streader_read_tstamp(sr, length))
        return false;

    if (Tstamp_get_beats(length) < 0)
    {
        Streader_set_error(sr, "Pattern length is negative");
        return false;
    }

    Tstamp_copy(&pat->length, length);

    return true;
}


void Pattern_set_inst_existent(Pattern* pat, int index, bool existent)
{
    rassert(pat != NULL);
    rassert(index >= 0);
    rassert(index < KQT_PAT_INSTANCES_MAX);

    Bit_array_set(pat->existents, index, existent);

    return;
}


bool Pattern_get_inst_existent(const Pattern* pat, int index)
{
    rassert(pat != NULL);
    rassert(index >= 0);
    rassert(index < KQT_PAT_INSTANCES_MAX);

    return Bit_array_get(pat->existents, index);
}


bool Pattern_set_column(Pattern* pat, int index, Column* col)
{
    rassert(pat != NULL);
    rassert(index >= 0);
    rassert(index < KQT_COLUMNS_MAX);
    rassert(col != NULL);

    Column* old_col = pat->cols[index];
    pat->cols[index] = col;
    del_Column(old_col);

    return true;
}


Column* Pattern_get_column(const Pattern* pat, int index)
{
    rassert(pat != NULL);
    rassert(index >= 0);
    rassert(index < KQT_COLUMNS_MAX);

    return pat->cols[index];
}


const Tstamp* Pattern_get_length(const Pattern* pat)
{
    rassert(pat != NULL);
    return &pat->length;
}


void del_Pattern(Pattern* pat)
{
    if (pat == NULL)
        return;

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
        del_Column(pat->cols[i]);

    del_Bit_array(pat->existents);
    memory_free(pat);

    return;
}


