

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
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
#include <debug/assert.h>
#include <memory.h>
#include <sheet/Pattern.h>
#include <string_common.h>


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


typedef struct pat_params
{
    Tstamp len;
} pat_params;

static bool read_pat_param(Streader* sr, const char* key, void* userdata)
{
    assert(sr != NULL);
    assert(key != NULL);
    assert(userdata != NULL);

    pat_params* pp = userdata;

    if (string_eq(key, "length"))
    {
        if (!Streader_read_tstamp(sr, &pp->len))
            return false;

        if (Tstamp_get_beats(&pp->len) < 0)
        {
            Streader_set_error(sr, "Pattern length is negative");
            return false;
        }
    }
    else
    {
        Streader_set_error(sr, "Unrecognised key in pattern header: %s", key);
        return false;
    }

    return true;
}

bool Pattern_parse_header(Pattern* pat, Streader* sr)
{
    assert(pat != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    pat_params* pp = &(pat_params)
    {
        .len = *PATTERN_DEFAULT_LENGTH,
    };

    if (!Streader_read_dict(sr, read_pat_param, pp))
        return false;

    Pattern_set_length(pat, &pp->len);

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


