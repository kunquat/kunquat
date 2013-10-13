

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <Bit_array.h>
#include <Connections_search.h>
#include <Event.h>
#include <memory.h>
#include <Pattern.h>
#include <xassert.h>


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


bool Pattern_parse_header(Pattern* pat, char* str, Read_state* state)
{
    assert(pat != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    Tstamp* len = PATTERN_DEFAULT_LENGTH;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        str = read_const_string(str, "length", state);
        str = read_const_char(str, ':', state);
        str = read_tstamp(str, len, state);
        str = read_const_char(str, '}', state);
        if (state->error)
            return false;
    }

    if (Tstamp_get_beats(len) < 0)
    {
        Read_state_set_error(state, "Pattern length is negative");
        return false;
    }

    Pattern_set_length(pat, len);

    return true;
}


void Pattern_set_inst_existent(Pattern* pat, int index, bool existent)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_PAT_INSTANCES_MAX);

    Bit_array_set(pat->existents, index, existent);

    return;
}


bool Pattern_get_inst_existent(const Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_PAT_INSTANCES_MAX);

    return Bit_array_get(pat->existents, index);
}


bool Pattern_set_column(Pattern* pat, int index, Column* col)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(col != NULL);

    Column* old_col = pat->cols[index];
    pat->cols[index] = col;
    del_Column(old_col);

    return true;
}


Column* Pattern_get_column(const Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);

    return pat->cols[index];
}


void Pattern_set_length(Pattern* pat, Tstamp* length)
{
    assert(pat != NULL);
    assert(length != NULL);
    assert(length->beats >= 0);
    Tstamp_copy(&pat->length, length);
    return;
}


const Tstamp* Pattern_get_length(const Pattern* pat)
{
    assert(pat != NULL);
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


